from base import *

def readTrajsFromDaruFile(fName):
    """ Read trajectory points from a text file.
    
    Each line of the file contains one point in the following format
    (angled braces provided for clarity).
    <trajID> <timestamp> <lat> <lon>
    
    Args:
        fName (str): name of the file.
        
    Returns:
        A dictionary of trajectories of the form {trID : traj}.
        Points within a trajectory are sorted by timestamp. 
    """
    
    trajs = {}    

    f = open(fName, 'r')
    for line in f:
        if "#" in line: continue
        line = line.replace("\n"," ")
        linetokens = line.split(" ")
        trajID = int(linetokens[0]);        
        t = float(linetokens[1]);
        lat = float(linetokens[2]);
        lon = float(linetokens[3]);
        if trajID not in trajs:
            trajs[trajID] = traj()
        trajs[trajID].addPt(lat, lon, trajID, t)

    f.close()

    for trajID in trajs:
        trajs[trajID].sortPts()            

    return trajs
    

def readTrajsFromTxtFile(fName):
    """ Read trajectory points from a text file.
    
    Each line of the file contains one point in the following format
    (angled braces provided for clarity).
    <trajID>;<objID>;<timestamp>;<lat>;<lon>
    objID is ignored.
    
    Args:
        fName (str): name of the file.
        
    Returns:
        A dictionary of trajectories of the form {trID : traj}.
        Points within a trajectory are sorted by timestamp. 
    """
    
    trajs = {}    

    f = open(fName, 'r')
    for line in f:
        if "#" in line: continue
        line = line.replace("\n",";")
        linetokens = line.split(";")
        trajID = int(linetokens[0]);        
        t = float(linetokens[2]);
        lat = float(linetokens[3]);
        lon = float(linetokens[4]);
        if trajID not in trajs:
            trajs[trajID] = traj()
        trajs[trajID].addPt(lat, lon, trajID, t)

    f.close()

    for trajID in trajs:
        trajs[trajID].sortPts()            

    return trajs
    
    
def readTrajsFromUmnTxtFile(fName):
    """ Read trajectory points from a text file generated by the
    U. Minn. traffic generator web tool.
    
    Each line of the file contains one point in the following format
    (angled braces provided for clarity).
    <trajID> <timestamp> <type> <lat> <lon>
    
    Args:
        fName (str): name of the file.
        
    Returns:
        A dictionary of trajectories of the form {trID : traj}.
        Points within a trajectory are sorted by timestamp. 
    """
    
    trajs = {}
    
    f = open(fName, 'r')
    counter = 0
    for line in f:
        if counter == 0: # ignore first line.
            counter = 1
            continue
        
        lineTokens = line.split()
        if len(lineTokens) != 5:
            continue
        trajID = int(lineTokens[0])
        t = float(lineTokens[1])
        lat = float(lineTokens[3])
        lon = float(lineTokens[4])
        if trajID not in trajs:
            trajs[trajID] = traj()
        trajs[trajID].addPt(lat, lon, trajID, t)
        
    f.close()
    
    for trajID in trajs:
        trajs[trajID].sortPts()
        
    return(trajs)
    
    
def writeTrajsToTxtFile(fName, trajs, wFlag=True):
    """ Write trajectory points to a text file.
    
    Each line of the file will contain one point in the following format
    (angled brackets provided for clarity).
    <trajID>;0;<time>;<lat>;<lon>
    
    Args:
        fName (str): name of the file.
        trajs: a dictionary of trajectories of the form {trID : traj}.
        wFlag (bool): if True, write to file; else print to stdout.
         
    """
    
    f = open(fName, 'w')
    for trajID in trajs:
        pts = trajs[trajID].pts
        for p in pts:
            tstr = str(p.t)
            s = str(trajID)+";0;"+ tstr +";"+str(p.lat)+";"+str(p.lon)
            if wFlag:
                f.write(s)
            else:
                print(s)
            if p != pts[-1] and wFlag:
                f.write("\n")
        f.write("\n")

    f.close()
    

def removeDuplicatePoints(tr):
    """ Remove duplicate points from a trajectory.
    
    Necessary because data read from files might contain duplicate points.
    
    Args:
        tr : traj object.
        
    """
    
    pts = tr.pts
    pts = list(set(pts))
    tr.pts = pts
    tr.sortPts()
