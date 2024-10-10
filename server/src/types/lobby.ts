import { Account } from "./account"

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