"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const socket_io_1 = require("socket.io");
const http_1 = require("http");
const express_1 = __importDefault(require("express"));
const socketTypes_1 = require("./socketTypes");
const app = (0, express_1.default)();
const httpServer = (0, http_1.createServer)(app);
const io = new socket_io_1.Server(httpServer);
let lobbies = {};
let swaps = {};
function broadcastLobbyUpdate(lobbyCode) {
    io.of(`/${lobbyCode}`).emit(socketTypes_1.Packet.LobbyUpdatedPacket, { info: lobbies[lobbyCode] });
}
// https://stackoverflow.com/a/7228322
function generateCode() {
    // return Math.floor(Math.random() * (999_999) + 1).toString().padStart(6, "0")
    return "000001"; // this is so that i dont suffer
}
function disconnectFromLobby(socket) {
    const { currentLobbyCode, account } = socket.data;
    if (currentLobbyCode == "")
        return;
    if (!account)
        return;
    let isDeletingLobby = false;
    if (Object.keys(lobbies).includes(currentLobbyCode)) {
        const index = lobbies[currentLobbyCode].accounts.map(e => e.userID).indexOf(account.userID);
        lobbies[currentLobbyCode].accounts.splice(index, 1);
        if (lobbies[currentLobbyCode].accounts.length == 0) {
            delete lobbies[currentLobbyCode];
            isDeletingLobby = true;
        }
    }
    console.log(`disconnecting ${account.name} (${account.userID}) from lobby with code ${currentLobbyCode}`);
    if (!isDeletingLobby) {
        broadcastLobbyUpdate(currentLobbyCode);
    }
}
const DUMMY_LEVEL_DATA = "THIS IS DUMMY DATA I REPEAT THIS IS DUMMY DATA";
class Swap {
    constructor(lobbyCode) {
        this.lobbyCode = lobbyCode;
        this.lobby = lobbies[lobbyCode];
        this.currentTurn = 0;
        this.totalTurns = this.lobby.accounts.length * this.lobby.settings.turns;
        this.levels = [];
    }
    swap() {
        this.levels = Array(this.lobby.accounts.length).fill(DUMMY_LEVEL_DATA);
        this.currentTurn++;
        io.of(`/${this.lobbyCode}`).emit(socketTypes_1.Packet.TimeToSwapPacket);
    }
    addLevel(level, accIdx) {
        this.levels[accIdx] = level;
        console.log(this.levels);
        if (this.levels.includes(DUMMY_LEVEL_DATA))
            return;
        let levels = new Array(this.levels.length).fill("");
        this.levels.forEach((level, index) => {
            let lvlIdx = index + this.currentTurn;
            if (lvlIdx > this.levels.length - 1) {
                lvlIdx = lvlIdx % (this.levels.length - 1);
            }
            console.log(lvlIdx);
            levels[lvlIdx] = level;
        });
        io.of(`/${this.lobbyCode}`).emit(socketTypes_1.Packet.RecieveSwappedLevelPacket, { levels });
        if (this.currentTurn >= this.totalTurns) {
            this.swapEnded = true;
            io.of(`/${this.lobbyCode}`).emit(socketTypes_1.Packet.SwapEndedPacket);
            return;
        }
        this.scheduleNextSwap();
    }
    scheduleNextSwap() {
        if (this.swapEnded)
            return;
        console.log("scheduling swap for 10 seconds (THIS IS HARDCODED CHANGE THIS BEFORE RELEASE PLEASE I BEG OF YOU)");
        setTimeout(() => {
            console.log("swap time!");
            this.swap();
        }, 20_000); // TODO: make this the actual time, this is 20s
    }
}
const handlers = {
    2001: (socket, args) => {
        console.log(args);
        const newLobby = {
            code: generateCode(),
            accounts: [],
            settings: args
        };
        lobbies[newLobby.code] = newLobby;
        console.log(newLobby.code);
        console.log(lobbies);
        socket.emit(socketTypes_1.Packet.LobbyCreatedPacket, { info: newLobby });
    },
    2002: (socket, args) => {
        const { code, account } = args;
        if (!Object.keys(lobbies).includes(code)) {
            socket.emit(socketTypes_1.Packet.ErrorPacket, { error: `lobby with code '${code}' does not exist` });
            return;
        }
        lobbies[code].accounts.push(account);
        console.log(`user ${account.name} has joined lobby ${lobbies[code].settings.name}`);
        socket.data.currentLobbyCode = code;
        socket.data.account = args.account;
        console.log(account);
        socket.emit(socketTypes_1.Packet.JoinedLobbyPacket);
        broadcastLobbyUpdate(code);
    },
    2003: (socket, args) => {
        const { code } = args;
        if (!Object.keys(lobbies).includes(code))
            return;
        socket.emit(socketTypes_1.Packet.RecieveAccountsPacket, { accounts: lobbies[code].accounts });
    },
    2004: (socket, args) => {
        const { code } = args;
        if (!Object.keys(lobbies).includes(code))
            return;
        socket.emit(socketTypes_1.Packet.RecieveLobbyInfoPacket, { info: lobbies[code] });
    },
    2005: (socket) => {
        disconnectFromLobby(socket);
    },
    2006: (socket, args) => {
        const { code } = args;
        if (!Object.keys(lobbies).includes(code))
            return;
        const { code: _, ...newArgs } = args;
        console.log(args);
        lobbies[code].settings = {
            ...lobbies[code].settings,
            ...newArgs
        };
        broadcastLobbyUpdate(code);
    },
    2007: (socket) => {
        const { currentLobbyCode: lobbyCode, account } = socket.data;
        if (!Object.keys(lobbies).includes(lobbyCode))
            return;
        if (lobbies[lobbyCode].settings.owner != parseInt(account.userID))
            return;
        let accs = [];
        lobbies[lobbyCode].accounts.forEach((account, index) => {
            console.log({ index, accID: account.userID });
            accs.push({
                index,
                accID: account.userID
            });
        });
        io.of(`/${lobbyCode}`).emit(socketTypes_1.Packet.SwapStartedPacket, { accounts: accs });
        swaps[lobbyCode] = new Swap(lobbyCode);
        swaps[lobbyCode].scheduleNextSwap();
    },
    3001: (socket, args) => {
        const { code } = args;
        console.log("hello?????");
        swaps[code].addLevel(args.lvlStr, args.accIdx);
    }
};
io.on("connection", (socket) => {
    console.log("we got ourselves a little GOOBER here\na professional FROLICKER");
    socket.on("packet", (args) => {
        if (!Object.keys(handlers).includes(args.packet_id.toString()))
            return;
        console.log(`[PACKET] handling packet ${args.packet_id}`);
        handlers[args.packet_id](socket, args);
    });
    socket.on("disconnect", (reason) => {
        disconnectFromLobby(socket);
    });
});
console.log("listening on port 3000");
httpServer.listen(3000);
