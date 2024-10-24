import { DBState } from "@/db/db"
import { Account } from "./account"
import { Lobby } from "./lobby"
import { Swap } from "./swap"
import WebSocket from "ws"

export interface SocketData {
    currentLobbyCode?: string
    account?: Account
    loggedIn?: boolean
}

export type ServerState = {
    lobbies: { [code: string]: Lobby }
    swaps : { [code: string]: Swap }
    kickedUsers : { [code: string]: number[] }
    sockets : { [code: string]: { [userID: number]: WebSocket } }
    dbState: DBState
}

export type LoginInfo = {
    version: string
    account: Account
}
