import WebSocket from "ws"
import { createServer } from "http"
import { default as express } from "express"

import * as socketTypes from "./socketTypes"

import {
    Lobby,
    PacketHandlers,
    Packet,
    Account
} from "./socketTypes"

const app = express()
const httpServer = createServer(app)

interface Swap {
    lobbyCode: string
    lobby: Lobby
    currentTurn: number

    levels: string[]

    totalTurns: number
    swapEnded: boolean

    timeout: NodeJS.Timeout
}

const wss = new WebSocket.Server({ server: httpServer })

let lobbies: { [code: string]: Lobby } = {}
let swaps : { [code: string]: Swap } = {}
let sockets : { [code: string]: WebSocket[] } = {}

function emitToLobby(lobbyCode: string, packetId: Packet, args: object) {
    sockets[lobbyCode]?.forEach((socket) => {
        sendPacket(socket, packetId, args)
    })
}

function broadcastLobbyUpdate(lobbyCode: string) {
    emitToLobby(lobbyCode, Packet.LobbyUpdatedPacket, { info: lobbies[lobbyCode] })
}


// https://stackoverflow.com/a/7228322
function generateCode() {
    return Math.floor(Math.random() * (999_999) + 1).toString().padStart(6, "0")
    // return "000001" // this is so that i dont suffer
}

function getLength(obj: any) {
    return Object.keys(obj).length
}

function disconnectFromLobby(data: socketTypes.SocketData) {
    const { currentLobbyCode: lobbyCode, account } = data;
    if (!lobbyCode) { console.log("could not find lobby"); return}
    if (!account) { console.log("could not find account"); return }
    let isDeletingLobby = false
    if (Object.keys(lobbies).includes(lobbyCode)) {
        const index = lobbies[lobbyCode].accounts.map(e => e.userID).indexOf(account.userID)
        lobbies[lobbyCode].accounts.splice(index, 1)
        if (getLength(lobbies[lobbyCode].accounts) == 0) {
            delete lobbies[lobbyCode]
            isDeletingLobby = true
        }
    }
    console.log(`disconnecting ${account.name} (${account.userID}) from lobby with code ${data.currentLobbyCode}`)
    if (isDeletingLobby && Object.keys(swaps).includes(lobbyCode)) {
        swaps[lobbyCode].unscheduleNextSwap()
        delete swaps[lobbyCode]
    }
    if (!isDeletingLobby) {
        broadcastLobbyUpdate(lobbyCode)
    }
}

const DUMMY_LEVEL_DATA = "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA"

function offsetArray(arr: string[], n: number): string[] {
    const length = arr.length;
    const result = new Array(length);

    for (let i = 0; i < length; i++) {
        // Calculate the new index using modulo to handle overflow
        const newIndex = (i + n) % length;
        result[newIndex] = arr[i];
    }

    return result;
}


class Swap {
    constructor(lobbyCode: string) {
        this.lobbyCode = lobbyCode
        this.lobby = lobbies[lobbyCode]
        this.currentTurn = 0

        this.totalTurns = getLength(this.lobby.accounts) * this.lobby.settings.turns

        this.levels = []
    }

    swap() {
        this.levels = Array(getLength(this.lobby.accounts)).fill(DUMMY_LEVEL_DATA)
        this.currentTurn++
        emitToLobby(this.lobbyCode, Packet.TimeToSwapPacket, {})
    }

    addLevel(level: string, accIdx: number) {
        this.levels[accIdx] = level
        console.log(this.levels)
        if (this.levels.includes(DUMMY_LEVEL_DATA)) return
        
        let levels: string[] = []
        if (this.levels.length == 2) {
            levels = this.levels.toReversed()
        } else {
            levels = offsetArray(this.levels, this.currentTurn)
        }
        console.log("a", levels)

        emitToLobby(this.lobbyCode, Packet.RecieveSwappedLevelPacket, { levels })

        console.log("current turn:", this.currentTurn)
        console.log("total turns (accs x settings.turns):", this.totalTurns)

        if (this.currentTurn >= this.totalTurns) {
            this.swapEnded = true
            setTimeout(() => emitToLobby(this.lobbyCode, Packet.SwapEndedPacket, {}), 750) // 0.75 seconds
            console.log("swap ended!")
            
            return
        }
        this.scheduleNextSwap()
    }

    scheduleNextSwap() {
        if (this.swapEnded) return
        console.log("scheduling swap for 10 seconds (THIS IS HARDCODED CHANGE THIS BEFORE RELEASE PLEASE I BEG OF YOU)")
        this.timeout = setTimeout(() => {
            console.log("swap time!")
            this.swap()
        }, 10_000) // TODO: make this the actual time, this is 20s
    }

    unscheduleNextSwap() {
        clearTimeout(this.timeout)
    }
}

function sendPacket(socket: WebSocket, packetId: Packet, args: socketTypes.ServerToClientEvents[typeof packetId]) {
    // someone's gonna cringe at this code
    let realArgs: any = {}
    if (Object.keys(args).length == 0) {
        realArgs = {
            // thank you cereal for not allowing
            // me to serialize nothing!
            dummy: ""
        }
    } else {
        realArgs = args
    }
    socket.send(`${packetId}|${JSON.stringify({ packet: realArgs })}`)
    console.log(`[PACKET] sent packet ${packetId}`)
}

function sendError(socket: WebSocket, error: string) {
    sendPacket(socket, Packet.ErrorPacket, { error })
}

const handlers: PacketHandlers = {
    2001: (socket, args) => { // CreateLobbyPacket (response: LobbyCreatedPacket)
        console.log(args)
        const newLobby: Lobby = {
            code: generateCode(),
            accounts: [],
            settings: args.settings
        }
        sockets[newLobby.code] = []
        lobbies[newLobby.code] = newLobby
        console.log(newLobby.code)
        console.log(lobbies)

        sendPacket(
            socket,
            Packet.LobbyCreatedPacket,
            {
                info: newLobby
            }
        )
    },
    2002: (socket, args, data) => { // JoinLobbyPacket
        const {code, account} = args
        if (!Object.keys(lobbies).includes(code)) {
            sendError(socket, `lobby with code '${code}' does not exist` )
            return
        }
        if (Object.keys(swaps).includes(code)) {
            sendError(socket, `creation rotation with code '${code}' is already in session` )
        }
        lobbies[code].accounts.push(account)
        sockets[code].push(socket)
        console.log(`user ${account.name} has joined lobby ${lobbies[code].settings.name}`)
        data.currentLobbyCode = code
        data.account = args.account

        console.log(data)

        sendPacket(socket, Packet.JoinedLobbyPacket, {})

        broadcastLobbyUpdate(code)
    },
    2003: (socket, args) => { // GetAccountsPacket (response: RecieveAccountsPacket)
        const { code } = args
        if (!Object.keys(lobbies).includes(code)) return
        sendPacket(socket, Packet.RecieveAccountsPacket, { accounts: lobbies[code].accounts })
    },
    2004: (socket, args) => { // GetLobbyInfoPacket (response: RecieveLobbyInfoPacket)
        const { code } = args
        if (!Object.keys(lobbies).includes(code)) return
        sendPacket(socket, Packet.RecieveLobbyInfoPacket, { info: lobbies[code] })
    },
    2005: (socket, _, data) => { // DisconnectFromLobbyPacket
        socket.close()
        // this is probably not needed anymore
        // disconnectFromLobby(data)
    },
    2006: (socket, args, data) => { // UpdateLobbyPacket
        const { code } = args
        if (!Object.keys(lobbies).includes(code)) {
            sendError(socket, "lobby doesn't exist")
            return
        }

        if (lobbies[code].settings.owner.toString() !== data.account?.userID) {
            sendError(socket, "you are not the owner of this lobby")
            return
        }

        const { code: _, ...newArgs } = args

        console.log(args)

        console.log("---")

        const oldSettings = lobbies[code].settings
        console.log(oldSettings)
        lobbies[code].settings = {
            ...oldSettings,
            ...newArgs.settings
        }

        console.log("new lobby")
        console.log(lobbies[code])

        broadcastLobbyUpdate(code)
    },
    2007: (socket, _, data) => { // StartSwapPacket
        const { currentLobbyCode: lobbyCode, account } = data
        // socket.

        if (!lobbyCode || !account) return

        if (!Object.keys(lobbies).includes(lobbyCode)) return
        if (lobbies[lobbyCode].settings.owner != parseInt(account.userID)) {
            sendError(socket, "you are not the owner of this lobby")
            return
        }
        if (lobbies[lobbyCode].accounts.length <= 1) {
            sendError(socket, "you are the only person in the lobby, cannot start level swap")
            return
        }

        let accs: Array<{ index: number, accID: string }> = []

        lobbies[lobbyCode].accounts.forEach((account, index) => {
            // const account = lobbies[lobbyCode].accounts[index]
            console.log({ index, accID: account.userID })
            accs.push({
                index: index,
                accID: account.userID
            })
        })
        emitToLobby(lobbyCode, Packet.SwapStartedPacket, { accounts: accs })

        swaps[lobbyCode] = new Swap(lobbyCode)
        swaps[lobbyCode].scheduleNextSwap()
    },
    3001: (socket, args) => { // SendLevelPacket
        const { code } = args
        console.log("hello?????")
        swaps[code].addLevel(args.lvlStr, args.accIdx)
    }
}

wss.on("connection", (socket) => {
    let data: socketTypes.SocketData = {}

    // i think it's time for more uh...
    // professional logs

    // console.log("we got ourselves a little GOOBER here\na professional FROLICKER")
    console.log(`new connection! ${socket.url}`)

    socket.on("message", (sdata) => {
        console.log(sdata.toString())
        const args = JSON.parse(sdata.toString())
        if (!args && !(typeof args === "object")) {
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
        handlers[packetId as any](socket, args["packet"], data)
    })

    socket.on("close", (code, reason) => {
        disconnectFromLobby(data)
    })
})

const port = process.env.PORT || 3000

console.log(`listening on port ${port}`)
httpServer.listen(port)