import { Lobby, Message } from "./lobby"
import { Account } from "./account"
import { LevelData, SwappedLevel } from "./swap"

export enum Packet {
    LobbyCreatedPacket = "1001",
    ReceiveAccountsPacket = "1002",
    ReceiveLobbyInfoPacket = "1003",
    LobbyUpdatedPacket = "1004",
    SwapStartedPacket = "1005",
    TimeToSwapPacket = "1006",
    JoinedLobbyPacket = "1007",
    ReceivePublicLobbiesPacket = "1008",
    MessageSentPacket = "1009",
    ReceiveSwappedLevelPacket = "3002",
    SwapEndedPacket = "3003",
    ErrorPacket = "4001",
    BannedUserPacket = "4002",
    AuthorizedUserPacket = "4003",
    ReceiveAuthCodePacket = "4004",
    ReceiveTokenPacket = "4005",
    InvalidTokenPacket = "4006",
    LoginNotReceivedPacket = "4007",
    LoggedInPacket = "4008",
    PongPacket = "4009"
}

export interface ServerToClientEvents {
    "1001": { info: Lobby } // LobbyCreatedPacket
    "1002": { accounts: Account[] } // ReceiveAccountsPacket
    "1003": { info: Lobby } // ReceiveLobbyInfoPacket
    "1004": { info: Lobby } // LobbyUpdatedPacket
    "1005": {} // SwapStartedPacket
    "1006": {} // TimeToSwapPacket
    "1007": {} // JoinedLobbyPacket
    "1008": { lobbies: Lobby[] } // ReceivePublicLobbiesPacket
    "1009": { message: Message } // MessageSentPacket
    "3002": { levels: { [key: number]: SwappedLevel } } // ReceiveSwappedLevelPacket
    "3003": {} // SwapEndedPacket
    "4001": { error: string } // ErrorPacket
    "4002": {} // BannedUserPacket
    "4003": {} // AuthorizedUserPacket
    "4004": { botAccID: number, code: string } // ReceiveAuthCodePacket
    "4005": { token: string } // ReceiveTokenPacket
    "4006": {} // InvalidTokenPacket
    "4007": {} // LoginNotReceivedPacket
    "4008": {} // LoggedInPacket
    "4009": {} // PongPacket
}