import { DBState } from "@/db/db"
import { Account } from "./account"
import { Lobby } from "./lobby"
import { Swap } from "./swap"
import { Config } from "@/config"

import WebSocket from "ws"
import { AuthManager } from "@/auth"

export interface SocketData {
    currentLobbyCode?: string
    account?: Account
    loggedIn?: boolean
    is_authorized: boolean
}

export type ServerState = {
    lobbies: { [code: string]: Lobby }
    swaps : { [code: string]: Swap }
    kickedUsers : { [code: string]: number[] }
    sockets : { [code: string]: { [userID: number]: WebSocket } }
    verifyCodes: { [accountID: number] : string }
    dbState: DBState
    serverConfig: Config
    socketCount: number
    peakSocketCount: number
    authManager?: AuthManager
}

export type LoginInfo = {
    version: string
    account: Account
    token: string
}
