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
    // password?: string
    minutesPerTurn: number
    isPublic: boolean // this will be a nightmare to add
}