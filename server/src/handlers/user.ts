import { Handlers } from "@/types/handlers"
import {
    sendError,
    sendPacket
} from "@/utils"
import { Packet } from "@/types/packet"
import { ServerState } from "@/types/state"

async function isModerator(state: ServerState, userID: number) {
    return (await state.dbState.getModeratorsIds()).includes(userID)
}

const userHandlers: Handlers = {
    5002: async (socket, args, data, state) => { // BanUserPacket
        if (!(await isModerator(state, data.account?.userID || 0))) {
            sendError(socket, "you are not a moderator")
            return
        }

        await state.dbState.banUser(state, data, args.user_id, args.reason)
        sendPacket(socket, Packet.BannedUserPacket, {})
    },
    5003: async (socket, args, data, state) => { // AuthorizeUserPacket
        if (!(await isModerator(state, data.account?.userID || 0))) {
            sendError(socket, "you are not a moderator")
            return
        }

        const result = await state.dbState.authorizeUser(data, args.password)
        if (!result) {
            sendError(socket, "unable to authorize; did you enter the correct password?")
            return
        }
        sendPacket(socket, Packet.AuthorizedUserPacket, {})
    }
}

export default userHandlers