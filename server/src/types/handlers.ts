import { Account } from "./account"
import { SocketData, ServerState, LoginInfo } from "./state"
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
    // GetAccountsPacket (response: ReceiveAccountsPacket)
    2003?: (socket: WebSocket, args: {
        code: string
    }, data: SocketData, state: ServerState) => void
    // GetLobbyInfoPacket (response: ReceiveLobbyInfoPacket)
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
    // GetPublicLobbiesPacket (response: ReceivePublicLobbiesPacket)
    2009?: (socket: WebSocket, args: {}, data: SocketData, state: ServerState) => void
    // SendMessagePacket
    2010?: (socket: WebSocket, args: { message: string }, data: SocketData, state: ServerState) => void
    // SendLevelPacket
    3001?: (socket: WebSocket, args: {
        code: string
        accIdx: number
        lvlStr: string
    }, data: SocketData, state: ServerState) => void
    // LoginPacket
    5001?: (socket: WebSocket, args: LoginInfo, data: SocketData, state: ServerState) => void
    // BanUserPacket
    5002?: (socket: WebSocket, args: { username: string, reason: string }, data: SocketData, state: ServerState) => void
    // AuthorizeUserPacket
    5003?: (socket: WebSocket, args: { password: string }, data: SocketData, state: ServerState) => void
    // RequestAuthorizationPacket
    5004?: (socket: WebSocket, args: { account_id: number }, data: SocketData, state: ServerState) => void 
    // VerifyAuthPacket
    5005?: (socket: WebSocket, args: { account: Account }, data: SocketData, state: ServerState) => void
    // UnbanUserPacket
    5006?: (socket: WebSocket, args: { account_id: number }, data: SocketData, state: ServerState) => void
    // PingPacket
    5007?: (socket: WebSocket, args: {}, data: SocketData, state: ServerState) => void

    [key: number]: Function | undefined
}