from helpers.std import *
from helpers.common import *

apply_data_template('tests/play_character/data/')

player = Client('player')

login_account(player, 'bobby', 'passw')
player.send('play bob')
player.expect('You have entered the world')
