# Example Python test script based on your JSON

from helpers.std import *

player = Client('player')

player.expect("Welcome")
player.send("create")
player.expect("username")
player.send("bobby")
player.expect("password")
player.send("passw")
player.expect("Confirm")
player.send("passw2")
player.expect("do not match")

file_not_exists("data/accounts/bobby.acct")
