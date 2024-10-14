import WebSocket from "ws"
import { createServer } from "http"
import { default as express } from "express"
import { version } from "../package.json"

import { Handlers } from "@/types/handlers"
import { SocketData, LoginInfo, ServerState } from "./types/state"
import { disconnectFromLobby, getLength } from "./utils"

import pako from "pako"

const app = express()
const httpServer = createServer(app)
const wss = new WebSocket.Server({ server: httpServer })

let handlers: Handlers = {}
let state: ServerState = {
    lobbies: {},
    kickedUsers: {},
    sockets: {},
    swaps: {}
}

const handlerFiles = ["lobby", "swap"]

handlerFiles.forEach(async (handlerName) => {
    const importedHandlers =
        (await import(`./handlers/${handlerName}`)).default as Handlers | undefined
    
    if (importedHandlers) {
        handlers = {...handlers, ...importedHandlers}
    } else {
        console.error(`[ERROR] unable to add handlers for file "${handlerName}". did you remember to use \`export default\`?`)
    }
})

wss.on("connection", (socket) => {
    let data: SocketData = {}

    console.log(`new connection! ${socket.url}`)

    socket.on("message", (sdata) => {
        if (sdata.toString().startsWith("login")) {
            const loginJson = JSON.parse(sdata.toString().split("|", 2)[1]) as LoginInfo
            if (!loginJson || typeof loginJson !== "object") {
                console.error("[LOGIN] recieved invalid login information")
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

        if (!data.loggedIn) {
            socket.close(
                1000,
                "did not recieve login data; cannot proceed"
            )
            return
        }

        const inflatedData = pako.inflate(sdata as Buffer, { to: "string" }).toString()
        const args = JSON.parse(inflatedData.toString())
        if (!args || typeof args !== "object") {
            console.error("[PACKET] recieved invalid packet string")
            return
        }   
        const packetId = args["packet_id"]

        if (!Object.keys(handlers).includes(String(packetId))) {
            console.log(`[PACKET] unhandled packet ${packetId}`)
            return
        }

        console.log(`[PACKET] handling packet ${packetId}`)

        // we love committing javascript war crimes
        const handlerFunc = handlers[packetId as any]
        if (handlerFunc) {
            handlerFunc(socket, args["packet"], data, state)
        }
    })

    socket.on("close", (code, reason) => {
        disconnectFromLobby(data, state)
    })

    socket.on("error", console.error)
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
            Number of connected clients: <b>${getLength(state.sockets)}</b>
        </p>
    `)
})

const port = process.env.PORT || 3000

console.log(`listening on port ${port}`)
httpServer.listen(port)