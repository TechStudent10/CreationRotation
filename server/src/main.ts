import { Server, Socket } from "socket.io"
import { createServer } from "http"
import { default as express } from "express"
import { v4 as uuidv4 } from "uuid"

import * as socketTypes from "./socketTypes"

import {
    Lobby,
    SocketConnection,
    PacketHandlers,
    Packet
} from "./socketTypes"

const app = express()
const httpServer = createServer(app)
const io = new Server<
    socketTypes.ClientToServerEvents,
    socketTypes.ServerToClientEvents,
    socketTypes.InterServerEvents,
    socketTypes.SocketData
>(httpServer)

let lobbies: { [code: string]: Lobby } = {}

interface Swap {
    lobbyCode: string
    lobby: Lobby
    currentTurn: number

    levels: string[]

    totalTurns: number
    swapEnded: boolean
}

let swaps : { [code: string]: Swap } = {}

function broadcastLobbyUpdate(lobbyCode: string) {
    io.of(`/${lobbyCode}`).emit(Packet.LobbyUpdatedPacket, { info: lobbies[lobbyCode] })
}

// https://stackoverflow.com/a/7228322
function generateCode() {
    // return Math.floor(Math.random() * (999_999) + 1).toString().padStart(6, "0")
    return "000001" // this is so that i dont suffer
}

function disconnectFromLobby(socket: SocketConnection) {
    const { currentLobbyCode, account } = socket.data;
    if (currentLobbyCode == "") return
    if (!account) return
    let isDeletingLobby = false
    if (Object.keys(lobbies).includes(currentLobbyCode)) {
        const index = lobbies[currentLobbyCode].accounts.map(e => e.userID).indexOf(account.userID)
        lobbies[currentLobbyCode].accounts.splice(index, 1)
        if (lobbies[currentLobbyCode].accounts.length == 0) {
            delete lobbies[currentLobbyCode]
            isDeletingLobby = true
        }
    }
    console.log(`disconnecting ${account.name} (${account.userID}) from lobby with code ${currentLobbyCode}`)
    if (!isDeletingLobby) {
        broadcastLobbyUpdate(currentLobbyCode)
    }
}

const DUMMY_LEVEL_DATA = "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA"

class Swap {
    constructor(lobbyCode: string) {
        this.lobbyCode = lobbyCode
        this.lobby = lobbies[lobbyCode]
        this.currentTurn = 0

        this.totalTurns = this.lobby.accounts.length * this.lobby.settings.turns

        this.levels = []
    }

    swap() {
        this.levels = Array(this.lobby.accounts.length).fill(DUMMY_LEVEL_DATA)
        this.currentTurn++
        io.of(`/${this.lobbyCode}`).emit(Packet.TimeToSwapPacket)
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
        io.of(`/${this.lobbyCode}`).emit(Packet.RecieveSwappedLevelPacket, { levels })

        if (this.currentTurn >= this.totalTurns) {
            this.swapEnded = true
            io.of(`/${this.lobbyCode}`).emit(Packet.SwapEndedPacket)
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

const handlers: PacketHandlers = {
    2001: (socket, args) => { // CreateLobbyPacket (response: LobbyCreatedPacket)
        console.log(args)
        const newLobby: Lobby = {
            code: generateCode(),
            accounts: [],
            settings: args
        }
        lobbies[newLobby.code] = newLobby
        console.log(newLobby.code)
        console.log(lobbies)

        socket.emit(Packet.LobbyCreatedPacket, { info: newLobby })
    },
    2002: (socket, args) => { // JoinLobbyPacket
        const {code, account} = args
        if (!Object.keys(lobbies).includes(code)) {
            socket.emit(Packet.ErrorPacket, { error: `lobby with code '${code}' does not exist` })
            return
        }
        lobbies[code].accounts.push(account)
        console.log(`user ${account.name} has joined lobby ${lobbies[code].settings.name}`)
        socket.data.currentLobbyCode = code
        socket.data.account = args.account

        console.log(account)

        socket.emit(Packet.JoinedLobbyPacket)

        broadcastLobbyUpdate(code)
    },
    2003: (socket, args) => { // GetAccountsPacket (response: RecieveAccountsPacket)
        const { code } = args
        if (!Object.keys(lobbies).includes(code)) return
        socket.emit(Packet.RecieveAccountsPacket, { accounts: lobbies[code].accounts })
    },
    2004: (socket, args) => { // GetLobbyInfoPacket (response: RecieveLobbyInfoPacket)
        const { code } = args
        if (!Object.keys(lobbies).includes(code)) return
        socket.emit(Packet.RecieveLobbyInfoPacket, { info: lobbies[code] })
    },
    2005: (socket) => { // DisconnectFromLobbyPacket
        disconnectFromLobby(socket)
    },
    2006: (socket, args) => { // UpdateLobbyPacket
        const { code } = args
        if (!Object.keys(lobbies).includes(code)) return

        const { code: _, ...newArgs } = args

        console.log(args)

        lobbies[code].settings = {
            ...lobbies[code].settings,
            ...newArgs
        }

        broadcastLobbyUpdate(code)
    },
    2007: (socket) => { // StartSwapPacket
        const { currentLobbyCode: lobbyCode, account } = socket.data

        if (!Object.keys(lobbies).includes(lobbyCode)) return
        if (lobbies[lobbyCode].settings.owner != parseInt(account.userID)) return

        let accs: Array<{ index: number, accID: string }> = []

        lobbies[lobbyCode].accounts.forEach((account, index) => {
            console.log({ index, accID: account.userID })
            accs.push({
                index,
                accID: account.userID
            })
        })
        io.of(`/${lobbyCode}`).emit(Packet.SwapStartedPacket, { accounts: accs })

        swaps[lobbyCode] = new Swap(lobbyCode)
        swaps[lobbyCode].scheduleNextSwap()
    },
    3001: (socket, args) => { // SendLevelPacket
        const { code } = args
        console.log("hello?????")
        swaps[code].addLevel(args.lvlStr, args.accIdx)
    }
}

io.on("connection", (socket) => {
    console.log("we got ourselves a little GOOBER here\na professional FROLICKER")

    socket.on("packet", (args) => {
        if (!Object.keys(handlers).includes(args.packet_id.toString())) return

        console.log(`[PACKET] handling packet ${args.packet_id}`)

        handlers[args.packet_id](socket, args)
    })

    socket.on("disconnect", (reason) => {
        disconnectFromLobby(socket)
    })
})

console.log("listening on port 3000")
httpServer.listen(3000)