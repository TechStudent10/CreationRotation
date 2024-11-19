import { Handlers } from "@/types/handlers"
import {
    sendError,
    sendPacket
} from "@/utils"
import { Packet } from "@/types/packet"
import { ServerState } from "@/types/state"
import { version } from "@/../package.json"
import log from "@/logging"
import { randomBytes } from "node:crypto"
import { Message } from "@/auth"

async function isModerator(state: ServerState, userID: number) {
    return (await state.dbState.getModeratorsIds()).includes(userID)
}

const userHandlers: Handlers = {
    5001: async (socket, args, data, state) => { // LoginPacket        
        let modVersion = args.version.replace("v", "")
        if (modVersion !== version) {
            socket.close(1000, `version mismatch: mod version <cy>${modVersion}</c> does not equal server version <cy>${version}</c>`)
            return
        }

        if (!(await state.dbState.isValidToken(args.account.accountID, args.token))) {
            socket.close(1000, "invalid token recieved")
            return
        }

        state.socketCount++
        if (state.socketCount > state.peakSocketCount) {
            state.peakSocketCount = state.socketCount
        }
        
        data.account = args.account
        data.loggedIn = true

        log.info(`new connection! ${data.account.name} (ID: ${data.account.userID}, connection #${state.socketCount})`)
    },
    5002: async (socket, args, data, state) => { // BanUserPacket
        if (!(await isModerator(state, data.account?.accountID || 0))) {
            sendError(socket, "you are not a moderator")
            return
        }

        if (await state.dbState.banUser(state, data, args.username, args.reason) == 0) {
            sendPacket(socket, Packet.BannedUserPacket, {})
        }
    },
    5003: async (socket, args, data, state) => { // AuthorizeUserPacket
        if (!(await isModerator(state, data.account?.accountID || 0))) {
            sendError(socket, "you are not a moderator")
            return
        }

        const result = await state.dbState.authorizeUser(data, args.password)
        if (!result) {
            sendError(socket, "unable to authorize; did you enter the correct password?")
            return
        }
        sendPacket(socket, Packet.AuthorizedUserPacket, {})
    },
    5004: async (socket, args, data, state) => { // RequestAuthorizationPacket
        const token = randomBytes(10).toString('hex')
        state.verifyCodes[args.account_id] = token
        sendPacket(socket, Packet.RecieveAuthCodePacket, { code: token, botAccID: state.serverConfig.botAccountID })
    },
    5005: async (socket, args, data, state) => { // VerifyAuthPacket
        state.authManager?.accountsToAuth.push({
            socket,
            account: args.account
        })
    },
    5006: async (socket, args, data, state) => { // UnbanUserPacket
        if (!(await isModerator(state, data.account?.accountID || 0))) {
            sendError(socket, "you are not a moderator")
            return
        }

        await state.dbState.unbanUser(args.account_id)
    },
}

export default userHandlers