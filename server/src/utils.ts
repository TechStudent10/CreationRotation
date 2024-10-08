import { Packet, ServerToClientEvents } from "@/types/packet"
import { SocketData, ServerState } from "@/types/state"
import WebSocket from "ws"

export function offsetArray(arr: string[], n: number): string[] {
    const length = arr.length;
    const result = new Array(length);

    for (let i = 0; i < length; i++) {
        // Calculate the new index using modulo to handle overflow
        const newIndex = (i + n) % length;
        result[newIndex] = arr[i];
    }

    return result;
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

            if (Object.keys(state.swaps).includes(lobbyCode)) {
                state.swaps[lobbyCode].unscheduleNextSwap()
                delete state.swaps[lobbyCode]
            }
            isDeletingLobby = true
        }
    }
    console.log(`disconnected ${account.name} (${account.userID}) from lobby with code ${data.currentLobbyCode}`)
    if (!isDeletingLobby) {
        broadcastLobbyUpdate(state, lobbyCode)
    }
}