import { Database, open } from "sqlite"
import sqlite3 from "sqlite3"

import * as path from "path"
import { path as appRoot } from "app-root-path"

import { ServerState } from "@/types/state"
import { SocketData } from "@/types/state"
import log from "@/logging"

import { hashPsw } from "@/utils"

import { randomBytes } from "node:crypto"
import { Account } from "@/types/account"

export const DB_PATH = path.join(__dirname, "database.db")

export interface DBState {
    db: Database
}

export type Moderator = {
    user_id: number
}

export type BannedUser = {

}

export class DBState {
    constructor() {
        this.openDB().then((db) => {
            this.db = db
            this.db.migrate({
                migrationsPath: path.join(appRoot, "migrations")
            })
            log.info("database migrated")
        })
    }

    private async openDB() {
        return open({
            filename: path.join(appRoot, "database.db"),
            driver: sqlite3.Database
        })
    }

    // -1 = you are not logged in
    // 0  = you are not a moderator
    // 1  = user banned!
    async banUser(state: ServerState, data: SocketData, username: string, reason: string) {
        if (!data.account) {
            return -1
        }

        if (!(await this.getModeratorsIds()).includes(data.account.accountID)) {
            return 0 // error
        }

        const response = await this.db.get(`SELECT account_id FROM users WHERE username LIKE ?`, username.toLowerCase())

        if (!response) {
            log.info("response failed :(")
            return -2
        }

        const { account_id } = response

        this.db.run(
            `
            INSERT INTO banned_users (account_id, issued_by, reason)
            VALUES (?, ?, ?)
            `,
            account_id,
            data.account.accountID,
            reason
        )

        return 1 // success!
    }

    async unbanUser(account_id: number) {
        this.db.run(
            `
            DELETE FROM banned_users
            WHERE account_id = ?
            `,
            account_id
        )
    }

    async promoteUser(account_id: number) {
        const password = randomBytes(10).toString("hex")

        await this.db.run(`
            INSERT INTO moderators (account_id, passw)
            VALUES (?, ?)
            `,
            account_id,
            hashPsw(password)
        )

        return password
    }

    async demoteUser(account_id: number) {
        if (!(await this.getModeratorsIds()).includes(account_id)) {
            return "Could not demote someone who is already demoted."
        }
        await this.db.run(
            `
            DELETE FROM moderators
            WHERE account_id = ?
            `,
            account_id
        )
        return "Demoted sucessfully"
    }

    async hasAuthenticated(accountID: number) {
        return await this.db.get("SELECT * FROM users WHERE account_id = ?", accountID) !== undefined
    }

    async registerUser(account: Account) {
        const token = randomBytes(30).toString("hex")

        await this.db.run(
            `
            INSERT INTO users (account_id, user_id, username, token)
            VALUES (?, ?, ?, ?)
            ON CONFLICT (account_id) DO
            UPDATE SET user_id = ?, username = ?, token = ?
            `,
            account.accountID, account.userID, account.name, token,
            account.userID, account.name, token
        )

        return token
    }

    async isValidToken(accountID: number, token: string) {
        // if we haven't even authenticated yet, the token is invalid
        if (!this.hasAuthenticated(accountID)) return false

        const response = await this.db.get(
            `
            SELECT token FROM users WHERE account_id = ?
            `,
            accountID
        )
        if (!response) return false

        const { token: acc_token } = response

        return token == acc_token
    }

    async authorizeUser(data: SocketData, password: string) {
        if (!(await this.getModeratorsIds()).includes(data.account?.accountID || 0)) {
            return false
        }

        const { passw: hashedPassw } = await this.db.get("SELECT passw FROM moderators WHERE account_id = ?", data.account?.accountID)
        data.is_authorized = hashPsw(password) === hashedPassw

        if (data.is_authorized) {
            log.log("ADMIN", `user ${data.account?.name} has logged in to the admin panel`)
        }

        return data.is_authorized
    }

    async getUsers() {
        return await this.db.all("SELECT * FROM users") as Array<{ user_id: number, username: string }>
    } 

    async getModeratorsIds() {
        return (await this.db.all("SELECT account_id FROM moderators")).map(val => val.account_id) as number[]
    }

    async getBannedIds() {
        return (await this.db.all("SELECT account_id FROM banned_users")).map(val => val.account_id) as number[]
    }
}
