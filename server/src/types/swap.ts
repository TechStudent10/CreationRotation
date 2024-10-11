import { Lobby } from "./lobby"
import {
    emitToLobby,
    getLength,
    offsetArray
} from "@/utils"
import { Packet } from "./packet"
import { ServerState } from "./state"

export interface Swap {
    lobbyCode: string
    lobby: Lobby
    currentTurn: number

    levels: string[]

    totalTurns: number
    swapEnded: boolean

    serverState: ServerState

    timeout: NodeJS.Timeout
}

const DUMMY_LEVEL_DATA = "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA"

export class Swap {
    constructor(lobbyCode: string, state: ServerState) {
        this.lobbyCode = lobbyCode
        this.lobby = state.lobbies[lobbyCode]
        this.currentTurn = 0
        this.serverState = state

        this.totalTurns = getLength(this.lobby.accounts) * this.lobby.settings.turns

        this.levels = []
    }

    swap() {
        this.levels = Array(getLength(this.lobby.accounts)).fill(DUMMY_LEVEL_DATA)
        this.currentTurn++
        emitToLobby(this.serverState, this.lobbyCode, Packet.TimeToSwapPacket, {})
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

        emitToLobby(this.serverState, this.lobbyCode, Packet.RecieveSwappedLevelPacket, { levels })

        console.log("current turn:", this.currentTurn)
        console.log("total turns (accs x settings.turns):", this.totalTurns)

        if (this.currentTurn >= this.totalTurns) {
            this.swapEnded = true
            setTimeout(() => emitToLobby(this.serverState, this.lobbyCode, Packet.SwapEndedPacket, {}), 750) // 0.75 seconds
            console.log("swap ended!")
            
            return
        }
        this.scheduleNextSwap()
    }

    scheduleNextSwap() {
        if (this.swapEnded) return
        this.timeout = setTimeout(() => {
            console.log("swap time!")
            this.swap()
        }, this.lobby.settings.minutesPerTurn * 1_000)
    }

    unscheduleNextSwap() {
        clearTimeout(this.timeout)
    }
}

