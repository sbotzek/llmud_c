from helpers.std import *

player = Client('player')
player2 = Client('player2')

player.expect("Welcome")
player.send("create")
player.expect("username")
player.send("bobby")
player.expect("password")
player.send("passw")
player.expect("Confirm")
player.send("passw")
player.expect("created")

player2.expect("Welcome")
player2.send("create")
player2.expect("username")
player2.send("bobby")
player2.expect("already in use")
