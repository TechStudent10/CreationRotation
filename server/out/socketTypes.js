"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Packet = void 0;
var Packet;
(function (Packet) {
    Packet["LobbyCreatedPacket"] = "1001";
    Packet["RecieveAccountsPacket"] = "1002";
    Packet["RecieveLobbyInfoPacket"] = "1003";
    Packet["LobbyUpdatedPacket"] = "1004";
    Packet["JoinedLobbyPacket"] = "1007";
    Packet["SwapStartedPacket"] = "1005";
    Packet["TimeToSwapPacket"] = "1006";
    Packet["RecieveSwappedLevelPacket"] = "3002";
    Packet["SwapEndedPacket"] = "3003";
    Packet["ErrorPacket"] = "4001";
})(Packet || (exports.Packet = Packet = {}));
