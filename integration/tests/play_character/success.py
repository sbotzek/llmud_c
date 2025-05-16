from helpers.std import *

apply_data_template('tests/play_character/data/')

player = Client('player')

player.expect('Welcome')
player.send('login')
player.expect('username')
player.send('bobby')
player.expect('password')
player.send('passw')
player.expect('Account menu:')
player.send('play bob')
player.expect('You have entered the world')
