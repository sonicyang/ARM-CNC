from uart_protocol import *
import math
import pdb 
############################
#   modified bt sonicyang  #
############################
################################################################################################
#                                                  #
#     G code interpreter and executer for 2D CNC laser engraver using Raspberry Pi             #
#       Xiang Zhai,   Oct 1, 2013                                              #
#       zxzhaixiang at gmail.com                                   #
#  For instruction on how to build the laser engraver and operate the codes, please visit      #
#   funofdiy.blogspot.com                                                                      #
#                                                  #
################################################################################################

import time
from numpy import pi, sin, cos, sqrt, arccos, arcsin

################################################################################################
################################################################################################
#################                            ###################################################
#################    Parameters set up       ###################################################
#################                            ###################################################
################################################################################################
################################################################################################

dx=0.3  #resolution in x direction. Unit: mm
dy=0.3  #resolution in y direction. Unit: mm
dz=1

mmdx = dx
mmdy = dy

indx = dx / 2.54
indy = dy / 2.54

#speed=10  #unit=mm/sec=0.04in/sec

curr_x_pos = 0
curr_y_pos = 0

abs_mode = 1

offset_x_pos = 0
offset_y_pos = 0

################################################################################################
################################################################################################
#################                                ###############################################
#################    G code reading Functions    ###############################################
#################                                ###############################################
################################################################################################
################################################################################################

def XYZFposition(lines):
    lines += " "
    x_pos = y_pos = z_pos = feedrate = "NOMOVE"
    try:
        #given a movement command line, return the X Y position
        xchar_loc=lines.index('X') 
        i=xchar_loc+1 
        while (47<ord(lines[i])<58)|(lines[i]=='.')|(lines[i]=='-'):
            i+=1 
        x_pos=float(lines[xchar_loc+1:i])     
    except:
        pass

    try:
        ychar_loc=lines.index('Y') 
        i=ychar_loc+1 
        while (47<ord(lines[i])<58)|(lines[i]=='.')|(lines[i]=='-'):
            i+=1 
        y_pos=float(lines[ychar_loc+1:i])     
    except:
        pass

    try:
        zchar_loc=lines.index('Z') 
        i=zchar_loc+1 
        while (47<ord(lines[i])<58)|(lines[i]=='.')|(lines[i]=='-'):
            i+=1 
        z_pos=float(lines[zchar_loc+1:i])     
    except:
        pass
    
    try:
        fchar_loc=lines.index('F') 
        i=fchar_loc+1 
        while (47<ord(lines[i])<58)|(lines[i]=='.')|(lines[i]=='-'):
            i+=1 
        feedrate=float(lines[fchar_loc+1:i])     
    except:
        pass

    return x_pos, y_pos, z_pos, feedrate

def IJposition(lines):
    #given a G02 or G03 movement command line, return the I J position
    ichar_loc=lines.index('I') 
    i=ichar_loc+1 
    while (47<ord(lines[i])<58)|(lines[i]=='.')|(lines[i]=='-'):
        i+=1 
    i_pos=float(lines[ichar_loc+1:i])     
    
    jchar_loc=lines.index('J') 
    i=jchar_loc+1 
    while (47<ord(lines[i])<58)|(lines[i]=='.')|(lines[i]=='-'):
        i+=1 
    j_pos=float(lines[jchar_loc+1:i])     

    return i_pos,j_pos 

def moveto(x_pos, y_pos, z_pos, feedrate):
    global curr_x_pos
    global curr_y_pos
    global curr_z_pos
    

    if(x_pos != "NOMOVE"):
        if abs_mode:
            x_pos -= curr_x_pos
        curr_x_pos += x_pos
    else:
        x_pos = int(0)

    if(y_pos != "NOMOVE"):
        if abs_mode:
            y_pos -= curr_y_pos
        curr_y_pos += y_pos
    else:
        y_pos = int(0)

    if(z_pos != "NOMOVE"):
        if z_pos > 0:
            z_pos = 1
        elif z_pos < 0:
            z_pos = -1
    else:
        z_pos = int(0) 
    
    if(feedrate == "NOMOVE"):
       feedrate = 0 
    
    feedrate /= dx;
    feedrate *= 2;

    #Translate mm into machine blocks
    x_pos /= dx
    y_pos /= dy

    parts = math.ceil(max(abs(x_pos), abs(y_pos)) / 200)
    if parts < 1:
        parts = 1   #for 0,0 bug fix
    
    x_pos /= parts
    y_pos /= parts
    
    x_error = x_pos - math.floor(x_pos) 
    y_error = y_pos - math.floor(y_pos) 

    x_pos = math.floor(x_pos) 
    y_pos = math.floor(y_pos)
    

    for i in range(parts):
       UART_Send_MOVE(x_pos + math.floor(i * x_error), y_pos + math.floor(i * y_error), z_pos, feedrate) 

    return 

###########################################################################################
###########################################################################################
#################                           ###############################################
#################    Main program           ###############################################
#################                           ###############################################
###########################################################################################
###########################################################################################

def printGCodeHelp():
    print("\nList of Aviliable Commands:")
    print("  G00 G01    Linear Movement")
    print("  G02 G03    Circle Movement")
    print("  G20        Set Unit in inchs")
    print("  G21        Set Unit in mm")
    print("  G90        Absolute Positioning Mode")
    print("  G91        Incremental Positioning Mode")
    print("  G92        Set Logical Origin Point")
    print("  M02        Quit Interactive Shell")
    print("  M100       Print Help Message")
    print("\n")

def ExcuteGCode(lines):
    global dx, dy, abs_mode, curr_x_pos, curr_y_pos

    if lines==[]:
        pass
    elif lines[0:3] == "G04":
        time.sleep(2); 
    elif lines[0:3]=='G90':
        abs_mode = 1
        print("Absolute Positioning mode")
        
    elif lines[0:3]=='G91':
        abs_moce = 0
        print("Incremental Positioning mode")

    elif lines[0:3]=='G92':
        curr_x_pos = 0
        curr_y_pos = 0
        print("Setting Logical Orin Point to Current Point")

    elif lines[0:3]=='G20':# working in inch 
        dx = indx
        dy = indy
        print('Working in inch')
          
    elif lines[0:3]=='G21':# working in mm 
        dx = mmdx
        dy = mmdy
        print('Working in mm')  
    elif lines[0:4] == "M100":
        printGCodeHelp()

    elif lines[0:3]=='M02':
        print('Finished!')

    elif (lines[0:3]=='G1F')|(lines[0:4]=='G1 F'):
        pass

    elif (lines[0:1]=='F'):

        [x_pos, y_pos, z_pos, feedrate]=XYZFposition(lines)
        moveto(x_pos, y_pos, z_pos, feedrate)

    elif (lines[0:3]=='G0 ')|(lines[0:3]=='G1 ')|(lines[0:3]=='G01')|(lines[0:3]=='G00')|(lines[0:1]==" "):#|(lines[0:3]=='G02')|(lines[0:3]=='G03'):
        
        [x_pos, y_pos, z_pos, feedrate]=XYZFposition(lines)
        moveto(x_pos, y_pos, z_pos, feedrate)
        print("Moving to ", x_pos, y_pos, z_pos, feedrate)
        
    elif (lines[0:3]=='G02')|(lines[0:3]=='G03'): #circular interpolation
        old_x_pos=curr_x_pos 
        old_y_pos=curr_y_pos 

        [x_pos,y_pos]=XYposition(lines) 
        [i_pos,j_pos]=IJposition(lines) 

        xcenter=old_x_pos+i_pos    #center of the circle for interpolation
        ycenter=old_y_pos+j_pos 
        
        
        Dx=x_pos-xcenter 
        Dy=y_pos-ycenter       #vector [Dx,Dy] points from the circle center to the new position
        
        r=sqrt(i_pos**2+j_pos**2)    # radius of the circle
        
        e1=[-i_pos,-j_pos]  #pointing from center to current position
        if (lines[0:3]=='G02'): #clockwise
            e2=[e1[1],-e1[0]]       #perpendicular to e1. e2 and e1 forms x-y system (clockwise)
        else:                   #counterclockwise
            e2=[-e1[1],e1[0]]       #perpendicular to e1. e1 and e2 forms x-y system (counterclockwise)

        #[Dx,Dy]=e1*cos(theta)+e2*sin(theta), theta is the open angle

        costheta=(Dx*e1[0]+Dy*e1[1])/r**2 
        sintheta=(Dx*e2[0]+Dy*e2[1])/r**2         #theta is the angule spanned by the circular interpolation curve
            
        if costheta>1:  # there will always be some numerical errors! Make sure abs(costheta)<=1
            costheta=1 
        elif costheta<-1:
            costheta=-1 

        theta=arccos(costheta) 
        if sintheta<0:
            theta=2.0*pi-theta 

        no_step=int(round(r*theta/dx/5.0))    # number of point for the circular interpolation
        
        for i in range(1,no_step+1):
            tmp_theta=i*theta/no_step 
            tmp_x_pos=xcenter+e1[0]*cos(tmp_theta)+e2[0]*sin(tmp_theta) 
            tmp_y_pos=ycenter+e1[1]*cos(tmp_theta)+e2[1]*sin(tmp_theta) 
            moveto(tmp_x_pos,tmp_y_pos) 
