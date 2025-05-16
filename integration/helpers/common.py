
def login_account(client, username, password):
    client.expect('Welcome')
    client.send('login')
    client.expect('username')
    client.send(username)
    client.expect('password')
    client.send(password)
    client.expect('Account menu:')

def login_character(client, username, password, character_name):
    login_account(client, username, password)
    client.send("play " + character_name + "")
    client.expect('You have entered the world')

