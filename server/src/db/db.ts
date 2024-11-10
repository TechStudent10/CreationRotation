import { open } from "sqlite"
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

export interface DBState {}

export type Moderator = {
    user_id: number
}

export type BannedUser = {

}

export class DBState {
    constructor() {
        this.openDB().then((db) => {
            db.migrate({
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
    async banUser(state: ServerState, data: SocketData, user_id: number, reason: string) {
        if (!data.account) {
            return -1
        }

        if (!(await this.getModeratorsIds()).includes(data.account.userID)) {
            return 0 // error
        }

        const db = await this.openDB()

        db.run(
            `
            INSERT INTO banned_users (user_id, issued_by, reason)
            VALUES (?, ?, ?)
            `,
            user_id,
            data.account.userID,
            reason
        )

        return 1 // success!
    }

    async hasAuthenticated(accountID: number) {
        const db = await this.openDB()
        return await db.get("SELECT * FROM users WHERE account_id = ?", accountID) !== undefined
    }

    async registerUser(account: Account) {
        const token = randomBytes(30).toString("hex")
        const db = await this.openDB()

        await db.run(
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

        const db = await this.openDB()

        const { token: acc_token } = await db.get(
            `
            SELECT token FROM users WHERE account_id = ?
            `,
            accountID
        )

        log.info(acc_token + " " + token)

        return token == acc_token
    }

    async loginUser(data: SocketData) {
        const users = await this.getUsers()
        const db = await this.openDB()
        if (users.findIndex(acc => acc.user_id === data.account?.userID) !== -1) {
            await db.run(
                `UPDATE users SET username = ? WHERE user_id = ?`,
                data.account?.name,
                data.account?.userID
            )
        } else {
            await db.run(
                `
                INSERT INTO users (user_id, username)
                VALUES (?, ?)
                `,
                data.account?.userID,
                data.account?.name
            )
        }
    }

    async authorizeUser(data: SocketData, password: string) {
        if (!(await this.getModeratorsIds()).includes(data.account?.userID || 0)) {
            return false
        }

        const db = await this.openDB()
        const { passw: hashedPassw } = await db.get("SELECT passw FROM moderators WHERE user_id = ?", data.account?.userID)
        data.is_authorized = hashPsw(password) === hashedPassw

        return data.is_authorized
    }

    async getUsers() {
        const db = await this.openDB()
        return await db.all("SELECT * FROM users") as Array<{ user_id: number, username: string }>
    } 

    async getModeratorsIds() {
        const db = await this.openDB()
        return (await db.all("SELECT user_id FROM moderators")).map(val => val.user_id) as number[]
    }
}
