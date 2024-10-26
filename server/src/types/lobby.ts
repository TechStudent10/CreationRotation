import { Account } from "./account"

export type Lobby = {
    settings: LobbySettings
    code: string
    accounts: Account[]
}

export type LobbySettings = {
    name: string
    turns: number
    owner: Account // owner user account
    // password?: string
    minutesPerTurn: number
    isPublic: boolean // this will be a nightmare to add
}