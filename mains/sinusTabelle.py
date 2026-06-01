import math

"""nur positive Sinushalbwelle -> Werte von 0 bis 1000"""
steps = 256
print('sinusTabelle = {') 
for i in range(0,steps):
    print( round(math.sin(math.pi * i/steps) *1000) ,end='')
    if i != steps-1:
        print(',',end='')
    if i%16 == 15:
        print('')
print('}')
