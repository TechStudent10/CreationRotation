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

export interface Swap {
    lobbyCode: string
    lobby: Lobby
    currentTurn: number

    levels: string[]

    totalTurns: number
    swapEnded: boolean
    swapOrder: number[]
    currentlySwapping: boolean

    accountIndexes: AccsWithIdx

    serverState: ServerState

    timeout: NodeJS.Timeout
}

const DUMMY_LEVEL_DATA = "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA"

export class Swap {
    constructor(lobbyCode: string, state: ServerState, accs: AccsWithIdx) {
        this.lobbyCode = lobbyCode
        this.lobby = state.lobbies[lobbyCode]
        this.currentTurn = 0
        this.serverState = state

        this.totalTurns = getLength(this.lobby.accounts) * this.lobby.settings.turns

        this.levels = []
        this.accountIndexes = accs

        this.currentlySwapping = false

        // initialize swap order
        this.swapOrder = offsetArray(
            [...Array(accs.length).keys()],
        1)
    }
    
    private swap() {
        this.levels = Array(getLength(this.swapOrder)).fill(DUMMY_LEVEL_DATA)
        this.currentTurn++
        this.currentlySwapping = true
        log.debug(this.swapOrder)
        emitToLobby(this.serverState, this.lobbyCode, Packet.TimeToSwapPacket, {})
    }

    addLevel(level: string, accIdx: number) {
        const idx = this.swapOrder.indexOf(accIdx)
        this.levels[idx] = level
        this.checkSwap()
    }

    checkSwap() {
        if (!this.currentlySwapping) return
        this.accountIndexes.forEach(acc => {
            if (this.serverState.lobbies[this.lobbyCode].accounts.findIndex(
                lobbyAcc => lobbyAcc.userID === acc.accID
            ) !== -1) return

            this.levels.splice(acc.index, 1)
        })
        if (this.levels.includes(DUMMY_LEVEL_DATA)) return
        this.currentlySwapping = false

        emitToLobby(this.serverState, this.lobbyCode, Packet.RecieveSwappedLevelPacket, { levels: this.levels })

        this.levels = []

        if (this.currentTurn >= this.totalTurns) {
            this.swapEnded = true
            setTimeout(() => emitToLobby(this.serverState, this.lobbyCode, Packet.SwapEndedPacket, {}), 750) // 0.75 seconds
            
            return
        }
        this.scheduleNextSwap()
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

