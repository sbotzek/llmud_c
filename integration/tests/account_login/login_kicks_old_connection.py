from helpers.std import *
from helpers.common import *

apply_data_template('tests/play_character/data/')

player = Client('player')
login_account(player, 'bobby', 'passw')

player2 = Client('player2')
login_account(player2, 'bobby', 'passw')

player2.expect('Previous session resumed.')
player.expect('Your account was logged into from another connection.')