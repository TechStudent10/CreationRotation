# Creation Rotation Server Protocol
This file lists off all the different packets and network details by the CR server.

The server uses `socket.io` for network communication, however the only time socket.io should be directly interfaced with on the client is when implementing `Packet::from_msg`, or when working on the internal managers.

## Packets

### Server
These packets are sent from the server to the client

- 1001 - LobbyCreatedPacket - sent when the server successfully creates a lobby. Contains lobby information.
- 1002 - RecieveAccountsPacket - response to GetAccountsPacket; contains accounts from a lobby.
- 1003 - RecieveLobbyInfoPacket - response to GetLobbyInfoPacket; contains lobby information.
- 1004 - LobbyUpdatedPacket - sent when either a lobby is updated by the owner or when a new member joins - emitted to a lobby namespace
- 1005 - SwapStartedPacket - sent when the owner starts a level swap; contains account IDs with their swap index - emitted to a lobby namespace
- 1006 - TimeToSwapPacket - sent when it's time for a lobby to swap levels - emitted to a lobby namespace
- 1007 - JoinedLobbyPacket - sent to the client when they successfully join a lobby
- 3002 - RecieveSwappedLevelPacket - sent when the server has swapped the levels; contains a list of levels - emitted to a lobby namespace
- 4001 - ErrorPacket - sent by the server containing information about some form of error; contains the error string

### Client
These packets are sent from the client to the server

- 2001 - CreateLobbyPacket - response: LobbyCreatedPacket - tells the server to create a new lobby
- 2002 - JoinLobbyPacket - response: JoinedLobbyPacket - joins a lobby
- 2003 - GetAccountsPacket - response: RecieveAccountsPacket - gets the accounts in a lobby
- 2004 - GetLobbyInfoPacket - response: RecieveLobbyInfoPacket - gets the information for a lobby
- 2005 - DisconnectFromLobbyPacket - disconnects the player from the lobby
- 2006 - UpdateLobbyPacket - sent from the owner of a lobby; updates the lobby serverside
- 2007 - StartSwapPacket - starts the swap in a lobby
- 3001 - SendLevelPacket - when it's time to swap, sends the level to server
