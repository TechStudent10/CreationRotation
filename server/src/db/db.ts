import { DatabaseSync } from "node:sqlite"
import * as path from "path"
import * as fs from "fs"

import { ServerState } from "@/types/state"
import { SocketData } from "@/types/state"

export const DB_PATH = path.join(__dirname, "database.db")

export interface DBState {
    dbconn: DatabaseSync
}

export type Moderator = {
    user_id: number
}

export type BannedUser = {

}

export class DBState {
    constructor() {
        let shouldInitDB = !fs.existsSync(DB_PATH)
        this.dbconn = new DatabaseSync(DB_PATH)
        if (shouldInitDB) {
            this.initDB()
        }
    }

    private initDB() {
        this.dbconn.exec(`
            CREATE TABLE banned_users (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL,
                issued_by INT NOT NULL,
                reason TEXT NOT NULL
            );

            CREATE TABLE moderators (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL
            );
        `)
    }

    // -1 = you are not logged in
    // 0  = you are not a moderator
    // 1  = user banned!
    banUser(state: ServerState, data: SocketData, user_id: number, reason: string): number {
        if (!data.account) {
            return -1
        }

        if (!this.getModeratorsIds().includes(data.account.userID)) {
            return 0 // error
        }

        this.dbconn.prepare(`
            INSERT INTO banned_users (user_id, issued_by, reason),
            VALUES (?, ?, ?)
        `).run(
            user_id,
            data.account.userID,
            reason
        )

        return 1 // success!
    }

    getModeratorsIds(): number[] {
        const query = this.dbconn.prepare("SELECT user_id FROM moderators")
        return query.all() as number[]
    }
}
