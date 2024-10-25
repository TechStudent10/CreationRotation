import log from "./logging"

export interface ErrorHandler {
    webhook_url: string
}

export class ErrorHandler {
    constructor(webhook_url: string) {
        this.webhook_url = webhook_url
    }

    notifyError(err: Error) {
        log.error(err.stack)

        fetch(
            this.webhook_url, {
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
        // todo: make exit finish all swaps
        process.on('exit', () => {})
        process.on('uncaughtException', (err) => {
            this.notifyError(err)
        })
        log.info("error handlers registered")
    }
}