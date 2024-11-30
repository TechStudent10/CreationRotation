import { Lobby, Message } from "./lobby"
import { Account } from "./account"

export enum Packet {
    LobbyCreatedPacket = "1001",
    RecieveAccountsPacket = "1002",
    RecieveLobbyInfoPacket = "1003",
    LobbyUpdatedPacket = "1004",
    SwapStartedPacket = "1005",
    TimeToSwapPacket = "1006",
    JoinedLobbyPacket = "1007",
    RecievePublicLobbiesPacket = "1008",
    MessageSentPacket = "1009",
    RecieveSwappedLevelPacket = "3002",
    SwapEndedPacket = "3003",
    ErrorPacket = "4001",
    BannedUserPacket = "4002",
    AuthorizedUserPacket = "4003",
    RecieveAuthCodePacket = "4004",
    RecieveTokenPacket = "4005",
    InvalidTokenPacket = "4006",
    LoginNotReceivedPacket = "4007",
    LoggedInPacket = "4008"
}

export interface ServerToClientEvents {
    "1001": { info: Lobby } // LobbyCreatedPacket
    "1002": { accounts: Account[] } // RecieveAccountsPacket
    "1003": { info: Lobby } // RecieveLobbyInfoPacket
    "1004": { info: Lobby } // LobbyUpdatedPacket
    "1005": { accounts: Array<{ index: number, accID: string }> } //SwapStartedPacket
    "1006": {} // TimeToSwapPacket
    "1007": {} // JoinedLobbyPacket
    "1008": { lobbies: Lobby[] } // RecievePublicLobbiesPacket
    "1009": { message: Message } // MessageSentPacket
    "3002": { levels: Array<string> } // RecieveSwappedLevelPacket
    "3003": {} // SwapEndedPacket
    "4001": { error: string } // ErrorPacket
    "4002": {} // BannedUserPacket
    "4003": {} // AuthorizedUserPacket
    "4004": { botAccID: number, code: string } // RecieveAuthCodePacket
    "4005": { token: string } // RecieveTokenPacket
    "4006": {} // InvalidTokenPacket
    "4007": {} // LoginNotReceivedPacket
    "4008": {} // LoggedInPacket
}