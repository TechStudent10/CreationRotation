import { Lobby } from "./lobby"
import { Account } from "./account"

export enum Packet {
    LobbyCreatedPacket = "1001",
    RecieveAccountsPacket = "1002",
    RecieveLobbyInfoPacket = "1003",
    LobbyUpdatedPacket = "1004",
    JoinedLobbyPacket = "1007",
    SwapStartedPacket = "1005",
    TimeToSwapPacket = "1006",
    RecievePublicLobbiesPacket = "1008",
    RecieveSwappedLevelPacket = "3002",
    SwapEndedPacket = "3003",
    ErrorPacket = "4001",
    BannedUserPacket = "4002",
    AuthorizedUserPacket = "4003",
    RecieveAuthCodePacket = "4004",
    RecieveTokenPacket = "4005"
}

export interface ServerToClientEvents {
    "1001": {info: Lobby}
    "1002": {accounts: Account[]}
    "1003": {info: Lobby}
    "1004": {info: Lobby}
    "1005": {accounts: Array<{ index: number, accID: string }>}
    "1006": {}
    "1007": {}
    "1008": {lobbies: Lobby[]}
    "3002": {levels: Array<string>}
    "3003": {}
    "4001": { error: string }
    "4002": {}
    "4003": {}
    "4004": { botAccID: number, code: string }
    "4005": { token: string }
}