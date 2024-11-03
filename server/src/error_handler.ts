import log from "./logging"
import { ServerState } from "./types/state"

export interface ErrorHandler {
    webhookUrl: string
    serverState: ServerState
}

export class ErrorHandler {
    constructor(webhook_url: string, state: ServerState) {
        this.webhookUrl = webhook_url
        this.serverState = state
    }

    notifyError(err: Error) {
        log.error(err.stack)

        fetch(
            this.webhookUrl, {
                method: "POST",
                body: JSON.stringify({
                    embeds: [
                        {
                            title: `gg server error! (\`${err.name}\`, \`${err.message}\`)`,
                            description: `\`\`\`javascript\n${err.stack}\`\`\`` || "no stack found"
                        }
                    ]
                }),
                headers: {
                    "Content-Type": "application/json"
                }
            }
        )
        .then(() => log.info("sent webhook"))
        .catch((reason) => log.error(`webhook not sent. reason: ${reason}`))
    }

    registerListeners() {
        const exitServ = () => {
            log.info("gracefully exitting server")
            process.exit()
        }

        // todo: make exit finish all swaps
        process.on('uncaughtException', (err) => {
            this.notifyError(err)
        })
        process.on("SIGINT", () => {
            log.info("got Control+C")
            exitServ()
        })
        process.on("SIGTERM", () => {
            log.info("Process killed")
            exitServ()
        })
        process.on("SIGHUP", () => {
            log.info("Process killed")
            exitServ()
        })
        process.on("SIGBREAK", () => {
            log.info("Process killed")
            exitServ()
        })
        log.info("error handlers registered")
    }
}