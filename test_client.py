import socket

def send_cmd(cmd):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 12345))
    s.send(cmd.encode() + b'\n')
    resp = s.recv(4096).decode()
    s.close()
    return resp

print("REGISTER:", send_cmd("REGISTER alice 123"))
print("LOGIN:", send_cmd("LOGIN alice 123"))
print("CHORD:", send_cmd("CHORD 1 3 0.0001"))
print("DES_ENCRYPT:", send_cmd("DES_ENCRYPT 12345678 Hello!!!"))
print("STEGANO_EMBED:", send_cmd('STEGANO_EMBED test_input.bmp test_output.bmp "Hello"'))
print("STEGANO_EXTRACT:", send_cmd("STEGANO_EXTRACT test_output.bmp"))
