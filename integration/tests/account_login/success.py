from helpers.std import *

apply_data_template('tests/account_login/data/')

player = Client('player')

player.expect('Welcome')
player.send('login')
player.expect('username')
player.send('bobby')
player.expect('password')
player.send('passw')
player.expect('Account menu:')