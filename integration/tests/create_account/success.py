from helpers.std import *

player = Client('player')

player.expect('Welcome')
player.send('create')
player.expect('username')
player.send('bobby')
player.expect('password')
player.send('passw')
player.expect('Confirm')
player.send('passw')
player.expect('created')

files_match("tests/create_account/success_account.acct", "data/accounts/bobby.acct")
