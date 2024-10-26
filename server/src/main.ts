import dotenv from "dotenv"
dotenv.config()

import WebSocket from "ws"
import { createServer } from "http"
import { default as express } from "express"
import { version } from "../package.json"

import { Handlers } from "@/types/handlers"
import { SocketData, LoginInfo, ServerState } from "./types/state"
import { disconnectFromLobby, getLength, sendError } from "./utils"

import pako from "pako"
import log from "./logging"
import { DBState } from "./db/db"
import { ErrorHandler } from "./error_handler"

const app = express()
const httpServer = createServer(app)
const wss = new WebSocket.Server({ server: httpServer })

let socketCount: number = 0
let peakSocketCount: number = 0

let handlers: Handlers = {}

const dbState = new DBState()

let state: ServerState = {
    lobbies: {},
    kickedUsers: {},
    sockets: {},
    swaps: {},
    dbState
}

const handlerFiles = ["lobby", "swap"]

handlerFiles.forEach(async (handlerName) => {
    try {
        const importedHandlers =
            (await import(`./handlers/${handlerName}`)).default as Handlers | undefined
        
        handlers = {...handlers, ...importedHandlers}
    } catch {
        log.error(`unable to add handlers for file "${handlerName}". did you remember to use \`export default\`?`)
    }
})

wss.on("connection", (socket) => {
    let data: SocketData = {};

    socket.on("message", (sdata) => {
        if (sdata.toString().startsWith("login")) {
            const loginJson = JSON.parse(sdata.toString().split("|", 2)[1]) as LoginInfo
            if (!loginJson || typeof loginJson !== "object") {
                log.log("login", `recieved invalid login information`)
                return
            }
            let modVersion = loginJson.version.replace("v", "")
            if (modVersion !== version) {
                socket.close(1000, `version mismatch: mod version <cy>${modVersion}</c> does not equal server version <cy>${version}</c>`)
                return
            }
            data.loggedIn = true
            return
        }

        let inflatedData: string;
        try {
            inflatedData = pako.inflate(sdata as Buffer, { to: "string" }).toString()
        } catch (e) {
            const errorStr = `error while attempting to decompress packet: ${e}`
            log.error(errorStr)
            sendError(socket, errorStr)
            return
        }

        const args = JSON.parse(inflatedData.toString())
        if (!args || typeof args !== "object") {
            log.packet("recieved invalid packet string")
            return
        }   
        const packetId = args["packet_id"]

        // handle the login packet first, it's special
        if (packetId === 5001) {
            const { packet: packetArgs } = args as { packet: LoginInfo }
            
            let modVersion = packetArgs.version.replace("v", "")
            if (modVersion !== version) {
                socket.close(1000, `version mismatch: mod version <cy>${modVersion}</c> does not equal server version <cy>${version}</c>`)
                return
            }

            socketCount++
            if (socketCount > peakSocketCount) {
                peakSocketCount = socketCount
            }
            
            data.account = packetArgs.account
            data.loggedIn = true

            log.info(`new connection! ${data.account.name} (ID: ${data.account.userID}, connection #${socketCount})`)

            return
        }

        if (!Object.keys(handlers).includes(String(packetId))) {
            log.packet(`unhandled packet ${packetId}`)
            return
        }

        if (!data.loggedIn) {
            socket.close(
                1000,
                "did not recieve login data; cannot proceed"
            )
            return
        }

        log.packet(`handling packet ${packetId}`)

        // we love committing javascript war crimes
        const handlerFunc = handlers[packetId as any]
        if (handlerFunc) {
            handlerFunc(socket, args["packet"], data, state)
        }
    })

    socket.on("close", (code, reason) => {
        if (!data.loggedIn) return
        socketCount--
        disconnectFromLobby(data, state)
    })

    socket.on("error", log.error)
})

app.get("/", (req, res) => {
    res.send("the server is up and running!")
})

app.get("/stats", (req, res) => {
    res.send(`
        <h1>Creation Rotation server statistics</h1>
        <p>
            Number of lobbies: <b>${getLength(state.lobbies)}</b>
            <br>
            Number of active swaps: <b>${getLength(state.swaps)}</b>
            <br>
            Lobbies subtract swaps (inactive swaps): <b>${getLength(state.lobbies) - getLength(state.swaps)}</b>
            <br>
            Number of connected clients: <b>${socketCount}</b>
            <br>
            Peak number of connected clients: <b>${peakSocketCount}</b>
        </p>
    `)
})

const port = process.env.PORT || 3000

const errHandler = new ErrorHandler(process.env.WEBHOOK_URL || "")
errHandler.registerListeners()

log.info(`listening on port ${port}`)
httpServer.listen(port)