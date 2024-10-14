import { Packet, ServerToClientEvents } from "@/types/packet"
import { SocketData, ServerState } from "@/types/state"
import WebSocket from "ws"

export function offsetArray(arr: any[], n: number): any[] {
    let array = [...arr]

    const len = array.length
    array.push(...array.splice(0, (-n % len + len) % len))
    return array
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
    console.log(`[PACKET] sent packet ${packetId}`)
}

export function sendError(socket: WebSocket, error: string) {
    sendPacket(socket, Packet.ErrorPacket, { error })
}


export function emitToLobby(state: ServerState, lobbyCode: string, packetId: Packet, args: object) {
    Object.values(state.sockets[lobbyCode])?.forEach((socket) => {
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
    if (!lobbyCode) { console.log("could not find lobby"); return}
    if (!account) { console.log("could not find account"); return }
    let isDeletingLobby = false
    if (Object.keys(state.lobbies).includes(lobbyCode)) {
        const index = state.lobbies[lobbyCode].accounts.map(e => e.userID).indexOf(account.userID)
        state.lobbies[lobbyCode].accounts.splice(index, 1)

        if (data.account?.userID === state.lobbies[lobbyCode].settings.owner) {
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
            } else {
                const accIdx = state.swaps[lobbyCode].accountIndexes.findIndex((acc) => acc.accID == data.account?.userID)
                const swapIdx = state.swaps[lobbyCode].swapOrder.indexOf(
                    state.swaps[lobbyCode].accountIndexes[accIdx].index
                )
                state.swaps[lobbyCode].swapOrder.splice(swapIdx, 1)
                
                if (state.swaps[lobbyCode].levels.length !== 0) {
                    state.swaps[lobbyCode].levels.splice(swapIdx, 1)
                }

                state.swaps[lobbyCode].checkSwap()
            }
        }
    }
    console.log(`disconnected ${account.name} (${account.userID}) from lobby with code ${data.currentLobbyCode}`)
    if (!isDeletingLobby) {
        broadcastLobbyUpdate(state, lobbyCode)
    }
}