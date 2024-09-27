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
    // return Math.floor(Math.random() * (999_999) + 1).toString().padStart(6, "0")
    return "000001" // this is so that i dont suffer
}

function getLength(obj: any) {
    return Object.keys(obj).length
}

function disconnectFromLobby(data: socketTypes.SocketData) {
    const { currentLobbyCode: lobbyCode, account } = data;
    // const account = data.account
    if (!lobbyCode) { console.log("could not find lobby"); return}
    if (!account) { console.log("could not find account"); return }
    let isDeletingLobby = false
    if (Object.keys(lobbies).includes(lobbyCode)) {
        const index = lobbies[lobbyCode].accounts.map(e => e.userID).indexOf(account.userID)
        lobbies[lobbyCode].accounts.splice(index, 1)
        // delete lobbies[lobbyCode].accounts[account.userID]
        if (getLength(lobbies[lobbyCode].accounts) == 0) {
            delete lobbies[lobbyCode]
            isDeletingLobby = true
        }
    }
    console.log(`disconnecting ${account.name} (${account.userID}) from lobby with code ${data.currentLobbyCode}`)
    if (!isDeletingLobby) {
        broadcastLobbyUpdate(lobbyCode)
    }
}

const DUMMY_LEVEL_DATA = "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA"

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
        let levels: string[] = new Array(this.levels.length).fill("")
        this.levels.forEach((level, index) => {
            let lvlIdx = index + this.currentTurn
            if (lvlIdx > this.levels.length - 1) {
                lvlIdx = lvlIdx % (this.levels.length - 1)
            }
            console.log(lvlIdx)
            levels[lvlIdx] = level
        })
        emitToLobby(this.lobbyCode, Packet.RecieveSwappedLevelPacket, { levels })

        if (this.currentTurn >= this.totalTurns) {
            this.swapEnded = true
            emitToLobby(this.lobbyCode, Packet.SwapEndedPacket, {})
            return
        }
        this.scheduleNextSwap()
    }

    scheduleNextSwap() {
        if (this.swapEnded) return
        console.log("scheduling swap for 10 seconds (THIS IS HARDCODED CHANGE THIS BEFORE RELEASE PLEASE I BEG OF YOU)")
        setTimeout(() => {
            console.log("swap time!")
            this.swap()
        }, 20_000) // TODO: make this the actual time, this is 20s
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
            sendPacket(socket, Packet.ErrorPacket, { error: `lobby with code '${code}' does not exist` })
            return
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
        // this is probably not needed anymore
        // disconnectFromLobby(data)
    },
    2006: (socket, args) => { // UpdateLobbyPacket
        const { code } = args
        if (!Object.keys(lobbies).includes(code)) return

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
        if (lobbies[lobbyCode].settings.owner != parseInt(account.userID)) return

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

    console.log("we got ourselves a little GOOBER here\na professional FROLICKER")

    socket.on("message", (sdata) => {
        const strData = sdata.toString().split("|", 2)
        const packetId = strData[0]
        const args = strData[1]

        if (!Object.keys(handlers).includes(packetId)) {
            console.log(`[PACKET] unhandled packet ${packetId}`)
            return
        }

        console.log(`[PACKET] handling packet ${packetId}`)

        // we love committing javascript war crimes
        handlers[packetId as any](socket, JSON.parse(args).packet, data)
    })

    socket.on("close", (code, reason) => {
        disconnectFromLobby(data)
    })
})

console.log("listening on port 3000")
httpServer.listen(3000)