from helpers.std import *

player = Client('player')
player2 = Client('player2')

player.expect("Welcome")
player2.expect("Welcome")

player.send("create")
player2.send("create")

player.expect("username")
player2.expect("username")

player.send("bobby")
player2.send("bobby")

player.expect("password")
player2.expect("password")

player.send("passw")
player2.send("passw2")

player.expect("Confirm")
player2.expect("Confirm")

player.send("passw")
player.expect("created")

player2.send("passw2")
player2.expect("already in use")
