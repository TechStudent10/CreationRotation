import { Socket } from "socket.io"

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
    "1001": (args: {info: Lobby}) => void
    "1002": (args: {accounts: Account[]}) => void
    "1003": (args: {info: Lobby}) => void
    "1004": (args: {info: Lobby}) => void
    "1005": (args: {accounts: Array<{ index: number, accID: string }>}) => void
    "1006": () => void
    "1007": () => void
    "3002": (args: {levels: Array<string>}) => void
    "3003": () => void
    "4001": (args: { error: string }) => void
}

export interface ClientToServerEvents {
    packet: (args: {
        packet_id: number
        [key: string]: any
    }) => void
}

export interface InterServerEvents {}
export interface SocketData {
    currentLobbyCode: string
    account: Account
}

export type SocketConnection = Socket<
    ClientToServerEvents,
    ServerToClientEvents,
    InterServerEvents,
    SocketData
>

export interface PacketHandlers {
    // CreateLobbyPacket (response: LobbyCreatedPacket)
    2001: (socket: SocketConnection, args: LobbySettings) => void
    // JoinLobbyPacket
    2002: (socket: SocketConnection, args: {
        code: string,
        account: Account
    }) => void
    // GetAccountsPacket (response: RecieveAccountsPacket)
    2003: (socket: SocketConnection, args: {
        code: string
    }) => void
    // GetLobbyInfoPacket (response: RecieveLobbyInfoPacket)
    2004: (socket: SocketConnection, args: {
        code: string
    }) => void
    // DisconnectFromLobbyPacket
    2005: (socket: SocketConnection) => void
    // UpdateLobbyPacket
    2006: (socket: SocketConnection, args: { code: string } & LobbySettings) => void
    // StartSwapPacket
    2007: (socket: SocketConnection) => void
    // SendLevelPacket
    3001: (socket: SocketConnection, args: {
        code: string
        accIdx: number
        lvlStr: string
    }) => void

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

