// import { Socket } from "socket.io"
import WebSocket from "ws"

// iconID, color1-3 should be integers but for
// the sake of maintaining sanity while working
// with C++ std::maps, im just gonna make them
// the same type.
export type Account = {
    name: string
    userID: string
    iconID: string
    color1: string
    color2: string
    color3: string
}


export type Lobby = {
    settings: LobbySettings
    code: string
    accounts: Account[]
}

export type LobbySettings = {
    name: string
    turns: number
    owner: number // owner user ID
    minutesPerTurn: number
}

export type CreationSwap = {
    lobby: Lobby
    currentTurn: number
}

export interface ServerToClientEvents {
    // packet: (args: {
    //     packet_id: number
    //     [key: string]: any
    // }) => {}
    "1001": {info: Lobby}
    "1002": {accounts: Account[]}
    "1003": {info: Lobby}
    "1004": {info: Lobby}
    "1005": {accounts: Array<{ index: number, accID: string }>}
    "1006": {}
    "1007": {}
    "3002": {levels: Array<string>}
    "3003": {}
    "4001": { error: string }
}

export interface ClientToServerEvents {
    packet: (args: {
        packet_id: number
        [key: string]: any
    }) => void
}

export interface InterServerEvents {}
export interface SocketData {
    currentLobbyCode?: string
    account?: Account
}

// export type SocketConnection = Socket<
//     ClientToServerEvents,
//     ServerToClientEvents,
//     InterServerEvents,
//     SocketData
// >

export interface PacketHandlers {
    // CreateLobbyPacket (response: LobbyCreatedPacket)
    2001: (socket: WebSocket, args: { settings: LobbySettings}, data: SocketData) => void
    // JoinLobbyPacket
    2002: (socket: WebSocket, args: {
        code: string,
        account: Account
    }, data: SocketData) => void
    // GetAccountsPacket (response: RecieveAccountsPacket)
    2003: (socket: WebSocket, args: {
        code: string
    }, data: SocketData) => void
    // GetLobbyInfoPacket (response: RecieveLobbyInfoPacket)
    2004: (socket: WebSocket, args: {
        code: string
    }, data: SocketData) => void
    // DisconnectFromLobbyPacket
    2005: (socket: WebSocket, args: {}, data: SocketData) => void
    // UpdateLobbyPacket
    2006: (socket: WebSocket, args: { code: string, settings: LobbySettings}, data: SocketData) => void
    // StartSwapPacket
    2007: (socket: WebSocket, args: {}, data: SocketData) => void
    // SendLevelPacket
    3001: (socket: WebSocket, args: {
        code: string
        accIdx: number
        lvlStr: string
    }, data: SocketData) => void

    [key: number]: Function
}

export enum Packet {
    LobbyCreatedPacket = "1001",
    RecieveAccountsPacket = "1002",
    RecieveLobbyInfoPacket = "1003",
    LobbyUpdatedPacket = "1004",
    JoinedLobbyPacket = "1007",
    SwapStartedPacket = "1005",
    TimeToSwapPacket = "1006",
    RecieveSwappedLevelPacket = "3002",
    SwapEndedPacket = "3003",
    ErrorPacket = "4001"
}

