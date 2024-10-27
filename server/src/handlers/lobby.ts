import { Handlers } from "@/types/handlers"
import {
    generateCode,
    sendPacket,
    sendError,
    broadcastLobbyUpdate
} from "@/utils"
import { Packet } from "@/types/packet"
import { Lobby } from "@/types/lobby"
import log from "@/logging"

import {
    RegExpMatcher,
    englishDataset,
    englishRecommendedTransformers
} from "obscenity"


const matcher = new RegExpMatcher({
    ...englishDataset.build(),
    ...englishRecommendedTransformers
})

function correctLobby(lobby: Lobby) {
    if (lobby.settings.minutesPerTurn > 60) {
        lobby.settings.minutesPerTurn = 60
    }
    if (lobby.settings.minutesPerTurn <= 0) {
        lobby.settings.minutesPerTurn = 1
    }
    if (lobby.settings.turns > 25) {
        lobby.settings.turns = 25
    }
    if (lobby.settings.turns <= 0) {
        lobby.settings.turns = 1
    }
}

const lobbyHandlers: Handlers = {
    2001: (socket, args, _, state) => { // CreateLobbyPacket (response: LobbyCreatedPacket)
        if (matcher.hasMatch(args.settings.name) && args.settings.isPublic) {
            sendError(socket, "the lobby name cannot contain profane terminology. please pick a different name.")
            return
        }

        const newLobby: Lobby = {
            code: generateCode(),
            accounts: [],
            settings: args.settings
        }
        correctLobby(newLobby)
        state.sockets[newLobby.code] = {}
        state.kickedUsers[newLobby.code] = []
        state.lobbies[newLobby.code] = newLobby


        sendPacket(
            socket,
            Packet.LobbyCreatedPacket,
            {
                info: newLobby
            }
        )
    },
    2002: (socket, args, data, state) => { // JoinLobbyPacket
        const { code } = args
        const { account } = data
        if (!account) {
            log.error("not logged in!")
            return
        }
        if (!Object.keys(state.lobbies).includes(code)) {
            sendError(socket, `lobby with code '${code}' does not exist` )
            return
        }
        if (Object.keys(state.swaps).includes(code)) {
            sendError(socket, `creation rotation with code '${code}' is already in session` )
            return
        }
        if (state.kickedUsers[code].includes(account.userID)) {
            sendError(socket, `you have been kicked from lobby <cy>"${state.lobbies[code].settings.name}"</c>. you cannot rejoin`)
            return
        }
        state.lobbies[code].accounts.push(account)
        state.sockets[code][account.userID] = socket
        log.info(`user ${account.name} has joined lobby ${state.lobbies[code].settings.name}`)
        data.currentLobbyCode = code

        sendPacket(socket, Packet.JoinedLobbyPacket, {})

        broadcastLobbyUpdate(state, code)
    },
    2003: (socket, _, data, state) => { // GetAccountsPacket (response: RecieveAccountsPacket)
        const { currentLobbyCode: code } = data
        if (!code) {
            sendError(socket, "you are not in a lobby")
            return
        }
        if (!Object.keys(state.lobbies).includes(code)) return
        sendPacket(socket, Packet.RecieveAccountsPacket, { accounts: state.lobbies[code].accounts })
    },
    2004: (socket, _, data, state) => { // GetLobbyInfoPacket (response: RecieveLobbyInfoPacket)
        const { currentLobbyCode: code } = data
        if (!code) {
            sendError(socket, "you are not in a lobby")
            return
        }
        if (!Object.keys(state.lobbies).includes(code)) return
        sendPacket(socket, Packet.RecieveLobbyInfoPacket, { info: state.lobbies[code] })
    },
    2005: (socket) => { // DisconnectFromLobbyPacket
        socket.close()
        // this is probably not needed anymore
        // disconnectFromLobby(data)
    },
    2006: (socket, args, data, state) => { // UpdateLobbyPacket
        if (matcher.hasMatch(args.settings.name) && args.settings.isPublic) {
            sendError(socket, "the lobby name cannot contain profane terminology. please pick a different name.")
            return
        }

        const { currentLobbyCode: code } = data
        if (!code) return
        if (!Object.keys(state.lobbies).includes(code)) {
            sendError(socket, "lobby doesn't exist")
            return
        }

        if (state.lobbies[code].settings.owner.userID !== data.account?.userID) {
            sendError(socket, "you are not the owner of this lobby")
            return
        }

        const { code: _, ...newArgs } = args

        const oldSettings = state.lobbies[code].settings
        state.lobbies[code].settings = {
            ...oldSettings,
            ...newArgs.settings
        }
        correctLobby(state.lobbies[code])

        broadcastLobbyUpdate(state, code)
    },
    2008: (socket, args, data, state) => { // KickUserPacket
        const { currentLobbyCode: lobbyCode, account } = data
        const { userID } = args

        if (!lobbyCode || !account) return
        
        if (!Object.keys(state.lobbies).includes(lobbyCode)) {
            sendError(socket, "invalid lobby code recieved")
            return
        }
        if (state.lobbies[lobbyCode].settings.owner.userID != account.userID) {
            sendError(socket, "you are not the owner of this lobby")
            return
        }
        if (account.userID == userID) {
            sendError(socket, "you cannot kick yourself")
            return
        }

        state.sockets[lobbyCode][userID].close(1000, "kicked from lobby by owner; you can no longer rejoin")
        state.kickedUsers[lobbyCode].push(userID)
    },
    2009: (socket, _, __, state) => { // GetPublicLobbiesPacket (response: RecievePublicLobbiesPacket)
        sendPacket(
            socket,
            Packet.RecievePublicLobbiesPacket,
            { lobbies: Object.values(state.lobbies)
                .filter((lobby) => lobby.settings.isPublic && !Object.keys(state.swaps).includes(lobby.code))
                .sort((a, b) => b.accounts.length - a.accounts.length) }
        )
    }
}

export default lobbyHandlers
