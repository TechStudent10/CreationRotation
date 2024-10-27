import { Account } from "./account"
import { SocketData, ServerState } from "./state"
import { LobbySettings } from "./lobby"

import WebSocket from "ws"

export type Handlers = {
    // CreateLobbyPacket (response: LobbyCreatedPacket)
    2001?: (
        socket: WebSocket,
        args: { settings: LobbySettings },
        data: SocketData,
        state: ServerState
    ) => void
    // JoinLobbyPacket
    2002?: (socket: WebSocket, args: {
        code: string,
        account: Account
    }, data: SocketData, state: ServerState) => void
    // GetAccountsPacket (response: RecieveAccountsPacket)
    2003?: (socket: WebSocket, args: {
        code: string
    }, data: SocketData, state: ServerState) => void
    // GetLobbyInfoPacket (response: RecieveLobbyInfoPacket)
    2004?: (socket: WebSocket, args: {
        code: string
    }, data: SocketData, state: ServerState) => void
    // DisconnectFromLobbyPacket
    2005?: (socket: WebSocket, args: {}, data: SocketData, state: ServerState) => void
    // UpdateLobbyPacket
    2006?: (
        socket: WebSocket,
        args: {
            code: string,
            settings: LobbySettings
        },
        data: SocketData,
        state: ServerState
    ) => void
    // StartSwapPacket
    2007?: (socket: WebSocket, args: {}, data: SocketData, state: ServerState) => void
    // KickUserPacket
    2008?: (socket: WebSocket, args: { userID: number }, data: SocketData, state: ServerState) => void
    // GetPublicLobbiesPacket (response: RecievePublicLobbiesPacket)
    2009?: (socket: WebSocket, args: {}, data: SocketData, state: ServerState) => void
    // SendLevelPacket
    3001?: (socket: WebSocket, args: {
        code: string
        accIdx: number
        lvlStr: string
    }, data: SocketData, state: ServerState) => void
    // LoginPacket (5001, handled in main.ts)
    // BanUserPacket
    5002?: (socket: WebSocket, args: { user_id: number, reason: string }, data: SocketData, state: ServerState) => void
    // AuthorizeUserPacket
    5003?: (socket: WebSocket, args: { password: string }, data: SocketData, state: ServerState) => void

    [key: number]: Function | undefined
}