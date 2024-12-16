import { Packet, ServerToClientEvents } from "@/types/packet"
import { SocketData, ServerState } from "@/types/state"
import log from "@/logging"

import WebSocket from "ws"
import { createHash } from "node:crypto"

export function offsetArray(arr: any[], n: number): any[] {
    let array = [...arr]

    const len = array.length
    array.push(...array.splice(0, (-n % len + len) % len))
    return array
}

export function hashPsw(password: string): string {
    return createHash("sha256").update(password).digest("base64")
}

export function sendPacket(socket: WebSocket, packetId: Packet, args: ServerToClientEvents[typeof packetId]) {
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
    log.packet(`sent packet ${packetId}`)
}

export function sendError(socket: WebSocket, error: string) {
    sendPacket(socket, Packet.ErrorPacket, { error })
}


export function emitToLobby(state: ServerState, lobbyCode: string, packetId: Packet, args: object) {
    if (!state.sockets[lobbyCode]) return
    Object.values(state.sockets[lobbyCode]).forEach((socket) => {
        sendPacket(socket, packetId, args)
    })
}

export function broadcastLobbyUpdate(state: ServerState, lobbyCode: string) {
    emitToLobby(state, lobbyCode, Packet.LobbyUpdatedPacket, { info: state.lobbies[lobbyCode] })
}

// https://stackoverflow.com/a/7228322
export function generateCode() {
    return Math.floor(Math.random() * (999_999) + 1).toString().padStart(6, "0")
    // return "000001" // this is so that i dont suffer
}

export function getLength(obj: any) {
    return Object.keys(obj).length
}

export function disconnectFromLobby(data: SocketData, state: ServerState) {
    const { currentLobbyCode: lobbyCode, account } = data;
    if (!lobbyCode) { log.error("could not find lobby"); return}
    if (!account) { log.error("could not find account"); return }
    let isDeletingLobby = false
    if (Object.keys(state.lobbies).includes(lobbyCode)) {
        const index = state.lobbies[lobbyCode].accounts.map(e => e.userID).indexOf(account.userID)
        state.lobbies[lobbyCode].accounts.splice(index, 1)

        if (data.account?.userID === state.lobbies[lobbyCode].settings.owner.userID) {
            Object.values(state.sockets[lobbyCode]).forEach((socket) => {
                socket.close(1000, "owner left, lobby closed")
            })
        }

        if (getLength(state.lobbies[lobbyCode].accounts) == 0) {
            delete state.lobbies[lobbyCode]
            delete state.kickedUsers[lobbyCode]
            delete state.sockets[lobbyCode]

            isDeletingLobby = true
        }
        if (Object.keys(state.swaps).includes(lobbyCode)) {
            if (isDeletingLobby) {
                state.swaps[lobbyCode].unscheduleNextSwap()
                delete state.swaps[lobbyCode]
            } else if (state.swaps[lobbyCode]) {
                if (getLength(state.swaps[lobbyCode].levels) !== 0) {
                    state.swaps[lobbyCode].checkSwap()
                }
            }
        }
    }
    log.info(`disconnected ${account.name} (${account.userID}) from lobby with code ${data.currentLobbyCode}`)
    if (!isDeletingLobby) {
        broadcastLobbyUpdate(state, lobbyCode)
    }
}