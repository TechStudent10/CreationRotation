import { ServerState } from "./types/state"

export interface AuthManager {
    state: ServerState
}

type Message = {
    messageID: number
    accountID: number
    playerID: number
    title: string
    content: string
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
        const messagesStr = (await this.sendAuthenticatedBoomlingsReq("database/getGJMessages20.php", {}))
            .split("|")
        
        let result: { [key: number] : Message } = {}
        messagesStr.forEach(async (messageStr) => {
            const msgID = parseKeyMap(messageStr)["1"]
            messageStr = await this.sendAuthenticatedBoomlingsReq("database/downloadGJMessage20.php", { messageID: msgID })
            const msgObj = parseKeyMap(messageStr)
            console.log(msgObj)
            result[msgID] = {
                accountID: parseInt(msgObj["2"]),
                age: msgObj["7"],
                content: this.xor(Buffer.from(msgObj["5"], "base64").toString("ascii"), "14251"),
                messageID: parseInt(msgID),
                playerID: parseInt(msgObj["3"]),
                title: Buffer.from(msgObj["4"], "base64").toString("ascii"),
                username: msgObj["6"]
            }
        })
        return result
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