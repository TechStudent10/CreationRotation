import * as fs from "fs"
import * as path from "path"
import appRootPath from "app-root-path"

export type Config = {
    webhookUrl?: string             // discord webhook URL
    boomlingsUrl: string            // GD server URL
    botAccountID: number            // authentication bot account ID
    botAccountGJP2: string          // authentication bot password (https://wyliemaster.github.io/gddocs/#/topics/gjp)
    masterPassword: string          // password used for promoting people
}

const configPath = path.join(appRootPath.path, "config.json")

export default function getConfig() {
    if (!fs.existsSync(configPath)) {
        const defaultConfig: Config = {
            webhookUrl: "",
            boomlingsUrl: "https://boomlings.com",
            botAccountID: 0,
            botAccountGJP2: "",
            masterPassword: "PLEASE_CHANGE_THIS"
        }

        fs.writeFileSync(configPath, JSON.stringify(defaultConfig, null, 4))
    }

    return JSON.parse(fs.readFileSync(configPath).toString()) as Config
}
