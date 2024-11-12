import { Account } from "./types/account"
import { ServerState } from "./types/state"
import WebSocket from "ws"
import { sendPacket } from "./utils"
import { Packet } from "./types/packet"
import log from "./logging"

export interface AuthManager {
    state: ServerState

    accountsToAuth: Array<{ socket: WebSocket, account: Account }>

    cachedMessages: { [key: number] : Message }
}

export type Message = {
    messageID: number
    accountID: number
    playerID: number
    title: string
    username: string
    age: string
}

// thanks prevter!
const parseKeyMap = (keyMap: string) => keyMap.split(":")
    .reduce((acc, key, index, array) => {
        if (index % 2 === 0) {
            acc[key] = array[index + 1];
        }
        return acc;
    }, ({} as {[key: string] : any}));

export class AuthManager {
    constructor(state: ServerState) {
        this.state = state
        this.accountsToAuth = []

        setInterval(() => {
            if (this.accountsToAuth.length <= 0) return
            
            this.updateMessagesCache()
        }, 4500) // 4500 = 4.5 seconds in milliseconds
    }

    private async sendBoomlingsReq(url: string, data: {[key: string] : string}, method: string = "POST") {
        return (await fetch(
            `${this.state.serverConfig.boomlingsUrl}/${url}`,
            {
                headers: {
                    "User-Agent": ""
                },
                method,
                body: new URLSearchParams({
                    secret: "Wmfd2893gb7",
                    ...data
                })
            }
        )).text()
    }

    private async sendAuthenticatedBoomlingsReq(url: string, data: {[key: string] : string}) {
        return await this.sendBoomlingsReq(url, {
            gjp2: this.state.serverConfig.botAccountGJP2,
            accountID: `${this.state.serverConfig.botAccountID}`,
            ...data
        })
    }

    async getMessages() {
        return this.cachedMessages
    }

    private async updateMessagesCache() {
        const messagesStr = (await this.sendAuthenticatedBoomlingsReq("database/getGJMessages20.php", {}))
            .split("|")
        
        log.info("refreshing cache")

        this.cachedMessages = {}
        messagesStr.forEach(async (messageStr) => {
            const msgObj = parseKeyMap(messageStr)
            const msgID = parseInt(msgObj["1"])
            this.cachedMessages[msgID] = {
                accountID: parseInt(msgObj["2"]),
                age: msgObj["7"],
                messageID: msgID,
                playerID: parseInt(msgObj["3"]),
                title: Buffer.from(msgObj["4"], "base64").toString("ascii"),
                username: msgObj["6"]
            }
        })

        let outdatedMessages: number[] = []

        Object.values(this.cachedMessages).forEach(async message => {
            this.accountsToAuth.forEach(async acc => {
                if (message.accountID !== acc.account.accountID) return

                if (message.title === this.state.verifyCodes[acc.account.accountID]) {
                    const token = await this.state.dbState.registerUser(acc.account)
                    sendPacket(acc.socket, Packet.RecieveTokenPacket, { token })
                    outdatedMessages.push(message.messageID)
                }
            })
        })

        this.accountsToAuth = []
        await this.sendAuthenticatedBoomlingsReq("database/deleteGJMessages20.php", {
            messages: outdatedMessages.join(",")
        })
    }

    async sendMessage(toAccID: number, subject: string, body: string) {
        return await this.sendAuthenticatedBoomlingsReq("database/uploadGJMessage20.php", {
            toAccountID: toAccID.toString(),
            subject: this.urlsafeb64(subject),
            body: this.urlsafeb64(this.xor(body, "14251"))
        })
    }

    // helper function for converting to URL-safe b64
    urlsafeb64(input: string) {
        return Buffer.from(input, "utf8").toString("base64")
    }

    // cycle xor algorithm from
    // https://wyliemaster.github.io/gddocs/#/topics/encryption/xor
    xor(input: string, key: string) {
        let result = ""
        const encoder = new TextEncoder()
        for (let i = 0; i < input.length; i++) {
            let byte = encoder.encode(input[i])[0]
            let xkey = encoder.encode(key[i % key.length])[0]
            result += String.fromCharCode(byte ^ xkey)
        }
        return result
    }
}