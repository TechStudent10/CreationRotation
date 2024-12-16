import { Lobby } from "./lobby"
import {
    emitToLobby,
    getLength,
    offsetArray
} from "@/utils"
import { Packet } from "./packet"
import { ServerState } from "./state"
import log from "@/logging"

export type AccsWithIdx = Array<{ index: number, accID: number }>

export type LevelData = {
    levelName: string
    songID: number
    songIDs: string
    levelString: string
}

export type SwappedLevel = {
    level: LevelData,
    accountID: number
}

export interface Swap {
    lobbyCode: string
    lobby: Lobby
    currentTurn: number

    levels: SwappedLevel[]

    totalTurns: number
    swapEnded: boolean
    swapOrder: number[]
    currentlySwapping: boolean
    isSwapEnding: boolean
    closeReason: string

    serverState: ServerState

    timeout: NodeJS.Timeout
}

const DUMMY_LEVEL_DATA: SwappedLevel = {
    level: {
        levelName: "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA",
        levelString: "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA",
        songID: 6942042069,
        songIDs: "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA"
    },
    accountID: 6942042069
}

export class Swap {
    constructor(lobbyCode: string, state: ServerState) {
        this.lobbyCode = lobbyCode
        this.lobby = state.lobbies[lobbyCode]
        this.currentTurn = 0
        this.serverState = state

        this.totalTurns = getLength(this.lobby.accounts) * this.lobby.settings.turns

        this.levels = []

        this.currentlySwapping = false
        this.isSwapEnding = false

        // initialize swap order
        this.swapOrder = this.lobby.accounts.map(acc => acc.accountID)
    }
    
    swap(ending: boolean = false, reason: string = "") {
        this.levels = []
        this.currentTurn++
        this.currentlySwapping = true
        this.isSwapEnding = ending
        this.closeReason = reason
        log.debug(this.swapOrder)
        emitToLobby(this.serverState, this.lobbyCode, Packet.TimeToSwapPacket, {})
    }

    addLevel(level: LevelData, accId: number) {
        const idx = this.swapOrder.indexOf(accId)
        this.levels.push(
            {
                accountID: parseInt(offsetArray(this.swapOrder, 1)[idx]),
                level
            }
        )
        this.checkSwap()
    }

    checkSwap() {
        if (!this.currentlySwapping) return
        this.lobby.accounts.forEach((acc, index) => {
            if (this.serverState.lobbies[this.lobbyCode].accounts.findIndex(
                lobbyAcc => lobbyAcc.accountID === acc.accountID
            ) !== -1) return

            this.levels.splice(index, 1)
        })
        if (getLength(this.levels) < this.lobby.accounts.length) return
        this.currentlySwapping = false

        if (!this.isSwapEnding) {
            emitToLobby(this.serverState, this.lobbyCode, Packet.ReceiveSwappedLevelPacket, { levels: this.levels })

            this.levels = []

            if (this.currentTurn >= this.totalTurns) {
                this.swapEnded = true
                setTimeout(() => emitToLobby(this.serverState, this.lobbyCode, Packet.SwapEndedPacket, {}), 750) // 0.75 seconds
                
                return
            }
            this.scheduleNextSwap()
        }
        // else {
        //     this.swapEnded = true
        //     this.levels = offsetArray(this.levels, this.totalTurns - this.currentTurn)
        //     emitToLobby(this.serverState, this.lobbyCode, Packet.ReceiveSwappedLevelPacket, { levels: this.levels })
        //     Object.values(this.serverState.sockets[this.lobbyCode]).forEach(socket => {
        //         socket.close(1000, this.closeReason)
        //     })
        // }
    }

    scheduleNextSwap() {
        if (this.swapEnded) return
        this.timeout = setTimeout(() => {
            this.swap()
        }, this.lobby.settings.minutesPerTurn * 60_000)
    }

    unscheduleNextSwap() {
        clearTimeout(this.timeout)
    }
}

