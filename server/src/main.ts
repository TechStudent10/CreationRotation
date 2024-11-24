import dotenv from "dotenv"
dotenv.config()

import WebSocket from "ws"
import { createServer } from "http"
import { default as express } from "express"

import { Handlers } from "@/types/handlers"
import { SocketData, LoginInfo, ServerState } from "./types/state"
import { disconnectFromLobby, getLength, hashPsw, sendError, sendPacket } from "./utils"

import pako from "pako"
import log from "./logging"
import { DBState } from "./db/db"
import { ErrorHandler } from "./error_handler"
import getConfig from "./config"
import { AuthManager } from "./auth"
import { Packet } from "./types/packet"

const app = express()
const httpServer = createServer(app)
const wss = new WebSocket.Server({ server: httpServer })

app.use(express.json())

let handlers: Handlers = {}

const dbState = new DBState()
let state: ServerState = {
    lobbies: {},
    kickedUsers: {},
    sockets: {},
    swaps: {},
    verifyCodes: {},
    serverConfig: getConfig(),
    socketCount: 0,
    peakSocketCount: 0,
    dbState
}
state.authManager = new AuthManager(state)

const handlerFiles = ["lobby", "swap", "user"]

handlerFiles.forEach(async (handlerName) => {
    try {
        const importedHandlers =
            (await import(`./handlers/${handlerName}`)).default as Handlers | undefined
        
        handlers = {...handlers, ...importedHandlers}
    } catch {
        log.error(`unable to add handlers for file "${handlerName}". did you remember to use \`export default\`?`)
    }
})

const unauthorizedPacketRange = [...Array(1000).keys()].map((val) => val + 5000)

wss.on("connection", (socket) => {
    let data: SocketData = {
        is_authorized: false
    }

    socket.on("message", (sdata) => {
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

        const doTheThing = () => {
            if (!Object.keys(handlers).includes(String(packetId))) {
                log.packet(`unhandled packet ${packetId}`)
                return
            }

            log.packet(`handling packet ${packetId}`)

            // we love committing typescript war crimes
            const handlerFunc = handlers[packetId as any]
            if (handlerFunc) {
                handlerFunc(socket, args["packet"], data, state)
            }
        }

        // handle user packets if we're not authorized
        if (unauthorizedPacketRange.includes(packetId)) {
            doTheThing()

            return
        }

        if (!data.loggedIn) {
            sendPacket(socket, Packet.LoginNotReceivedPacket, {})
            return
        }

        doTheThing()
    })

    socket.on("close", (code, reason) => {
        if (!data.loggedIn) return
        state.socketCount--
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
            Number of connected clients: <b>${state.socketCount}</b>
            <br>
            Peak number of connected clients: <b>${state.peakSocketCount}</b>
        </p>
    `)
})

app.post("/promote", async (req, res) => {
    if (req.body["password"] !== state.serverConfig.masterPassword) {
        res.send("not authenticated")
        return
    }

    if (req.body["account_id"]) {
        const passw = await state.dbState.promoteUser(req.body["account_id"])
        res.send(`password: ${passw}`)
    } else {
        res.send("no account_id found")
    }
})

app.post("/demote", async (req, res) => {
    if (req.body["password"] !== state.serverConfig.masterPassword) {
        res.send("not authenticated")
        return
    }

    res.send(await state.dbState.demoteUser(req.body["account_id"]))
})

const port = process.env.PORT || 3000

const errHandler = new ErrorHandler(state)
errHandler.registerListeners()

log.info(`listening on port ${port}`)
httpServer.listen(port)