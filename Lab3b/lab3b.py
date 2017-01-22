import csv

class Inode:
    def __init__(self):
        self.inodeNumber = 0
        self.ref_by_list = []
        self.nlinks = 0
        self.ptrs = []

    def addToList(self, element):
        self.ref_by_list.append(element)

    def getList(self):
        return self.ref_by_list(element)
    
    def __str__(self):
        return str(self.inodeNumber)+"\n"+str(self.ref_by_list)+"\n"+str(self.nlinks)+"\n"+str(self.ptrs)+"\n"

class Block:
    def __init__(self):
        self.blockNumber = 0
        self.ref_by_list = []

    def addToList(self, element):
        self.ref_by_list.append(element)

    def getList(self):
        return self.ref_by_list(element)
            
def main():
    inodeBitmapBlocks = []
    blockBitmapBlocks = []   
    inodeFreeList = []       # list of the free inode numbers
    blockFreeList = []       # list of the free block numbers
    inodeAllocated = {}      # key: inodeNum, value: inode class
    blockAllocated = {}      # key: blockNum, value: block class
    indirectTable = {}       # key: (blockNum, entryNum), value: ptr
    directoryTable = {}      # key: child inode, value: parent inode

    totalNumBlocks = 0
    blockSize = 0
    totalNumInodes = 0
    inodesPerGroup = 0

    outputFile = open("lab3b_check.txt", 'w')
    
    # Opens super.csv file and gets blockSize, total number of inode, and total number of blocks
    with open("super.csv", 'r') as supercsv:
        reader = csv.reader(supercsv)
        for row in reader:
            blockSize = int(row[3])
            totalNumBlocks = int(row[2])
            totalNumInodes = int(row[1])
            inodesPerGroup = int(row[6])

    # Opens group.csv file and puts content into inodeBitmapBlocks and blockBitmapBlocks
    with open("group.csv", 'r') as groupcsv:
        reader = csv.reader(groupcsv)
        for row in reader:
            inodeBitmapBlocks.append(int(row[4]))
            blockBitmapBlocks.append(int(row[5]))

    # makes indrectTable from the info in indirect.csv
    with open("indirect.csv", 'r') as indirectcsv:
        reader = csv.reader(indirectcsv)
        for row in reader:
            indirectTable[(int(row[0], 16) , int(row[1]))] = int(row[2], 16)
    
    # Open the bitmap.csv file and put its contents int inodeFreeList and blockFreeList
    with open("bitmap.csv", 'r') as bitmapcsv:
        reader = csv.reader(bitmapcsv)
        for row in reader:
            if int(row[0]) in inodeBitmapBlocks:
                inodeFreeList.append(int(row[1]))
            elif int(row[0]) in blockBitmapBlocks:
                blockFreeList.append(int(row[1]))


    # Creates inodeAllocated without the ref_by_list entry
    with open("inode.csv", 'r') as inodecsv:
        reader = csv.reader(inodecsv)
        for row in reader:
            inodeNumber = int(row[0])
            inode = Inode()
            inode.inodeNumber = inodeNumber
            inode.nlinks = int(row[5])
            for i in range(15):
                if int(row[i+11], 16) == 0:
                    break
                else:
                    inode.ptrs.append(int(row[i+11], 16))
            inodeAllocated[inodeNumber] = inode


            #begin
            # if the number of blocks is less than 12, we just look at the direct blocks
            numBlocks = int(row[10])
            if numBlocks <= 12:
                for i in range(numBlocks):
                    blockNum = int(row[i+11], 16)
                    if (blockNum == 0) or (blockNum > totalNumBlocks):
                        outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > ENTRY < " + i + " >\n")
                    else:
                        if blockNum in blockAllocated:
                            blockAllocated[blockNum].ref_by_list.append((int(row[0]), None, i))
                        else:
                            block = Block()
                            block.blockNumber = blockNum
                            block.ref_by_list.append((int(row[0]), None, i))
                            blockAllocated[blockNum] = block
            else:
                #do first 12
                for i in range(12):
                    blockNum = int(row[i+11], 16)
                    if (blockNum == 0) or (blockNum > totalNumBlocks):
                        outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > ENTRY < " + i + " >\n")
                    else:
                        block = Block()
                        block.blockNumber = blockNum
                        block.ref_by_list.append((int(row[0]), None, i))
                        blockAllocated[blockNum] = block
                #do rest
                #but first, do some math
                numLeft = len(indirectTable)
                #triply
                if numBlocks > 12 + 256 + (256*256):
                    #single
                    indirectNum = int(row[23], 16)
                    for j in range(256):
                        blockNum = indirectTable[indirectNum, j]
                        if blockNum == 0 or blockNum > totalNumBlocks:
                            outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > INDIRECT BLOCK < " + indirectNum + " > ENTRY < " + j + " >\n")
                        else:
                            block = Block()
                            block.blockNumber = blockNum;
                            block.ref_by_list.append((int(row[0]), blockNum, j))
                            blockAllocated[blockNum] = block
                        numLeft -= 1;

                    #double
                    doubleIndirectNum = int(row[24], 16)
                    for k in range(256):
                        indirectNum = indirectTable[doubleIndirectNum, k]
                        blockNum = indirectTable[doubleIndirectNum, k]
                        block = Block()
                        block.blockNumber = blockNum
                        block.ref_by_list.append((int(row[0]), blockNum, k))
                        blockAllocated[blockNum] = block
                        numLeft -= 1;
                        for j in range(256):
                            blockNum = indirectTable[indirectNum, j]
                            if blockNum == 0 or blockNum > totalNumBlocks:
                                outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > INDIRECT BLOCK < " + indirectNum + " > ENTRY < " + j + " >\n")
                            else:
                                block = Block()
                                block.blockNumber = blockNum
                                block.ref_by_list.append((int(row[0]), blockNum, j))
                                blockAllocated[blockNum] = block
                            numLeft -= 1;

                    #triple
                    tripleIndirectNum = int(row[25], 16)
                    for l in range(256):
                        if numLeft == 0:
                            break
                        doubleIndirectNum = indirectTable[tripleIndirectNum, l]
                        blockNum = indirectTable[tripleIndirectNum, l]
                        block = Block()
                        block.blockNumber = blockNum
                        block.ref_by_list.append((int(row[0]), blockNum, l))
                        blockAllocated[blockNum] = block
                        numLeft -= 1;
                        for k in range(256):
                            if numLeft == 0:
                                break
                            indirectNum = indirectTable[doubleIndirectNum, k]
                            blockNum = indirectTable[doubleIndirectNum, k]
                            block = Block()
                            block.blockNumber = blockNum
                            block.ref_by_list.append((int(row[0]), blockNum, k))
                            blockAllocated[blockNum] = block
                            numLeft -= 1;
                            for j in range(256):
                                if numLeft == 0:
                                    break
                                blockNum = indirectTable[indirectNum, j]
                                if blockNum == 0 or blockNum > totalNumBlocks:
                                    outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > INDIRECT BLOCK < " + indirectNum + " > ENTRY < " + j + " >\n")
                                else:
                                    block = Block()
                                    block.blockNumber = blockNum
                                    block.ref_by_list.append((int(row[0]), blockNum, j))
                                    blockAllocated[blockNum] = block
                                numLeft -= 1;


                #doubly
                elif numBlocks > 12 + 256:
                    #single
                    indirectNum = int(row[23], 16)
                    for j in range(256):
                        blockNum = indirectTable[indirectNum, j]
                        if blockNum == 0 or blockNum > totalNumBlocks:
                            outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > INDIRECT BLOCK < " + indirectNum + " > ENTRY < " + j + " >\n")
                        else:
                            block = Block()
                            block.blockNumber = blockNum
                            block.ref_by_list.append((int(row[0]), blockNum, j))
                            blockAllocated[blockNum] = block
                        numLeft -= 1;

                    #double
                    doubleIndirectNum = int(row[24], 16)
                    for k in range(256):
                        if numLeft == 0:
                            break
                        indirectNum = indirectTable[doubleIndirectNum, k]
                        blockNum = indirectTable[doubleIndirectNum, k]
                        block = Block()
                        block.blockNumber = blockNum
                        block.ref_by_list.append((int(row[0]), blockNum, k))
                        blockAllocated[blockNum] = block
                        numLeft -= 1;
                        for j in range(256):
                            if numLeft == 0:
                                break
                            blockNum = indirectTable[indirectNum, j]
                            if blockNum == 0 or blockNum > totalNumBlocks:
                                outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > INDIRECT BLOCK < " + indirectNum + " > ENTRY < " + j + " >\n")
                            else:
                                block = Block()
                                block.blockNumber = blockNum
                                block.ref_by_list.append((int(row[0]), blockNum, j))
                                blockAllocated[blockNum] = block
                            numLeft -= 1;


                #singly
                elif numBlocks > 12:
                    indirectNum = int(row[23], 16)
                    for j in range(numBlocks-12):
                        blockNum = indirectTable[indirectNum, j]
                        if blockNum == 0 or blockNum > totalNumBlocks:
                            outputFile.write("INVALID BLOCK < " + blockNum + " > IN INODE < " + int(row[0]) + " > INDIRECT BLOCK < " + indirectNum + " > ENTRY < " + j + " >\n")
                        else:
                            block = Block()
                            block.blockNumber = blockNum
                            block.ref_by_list.append((int(row[0]), blockNum, j))
                            blockAllocated[blockNum] = block
             #end
             
    # Make directoryTable from directory.csv
    # updates inodeAllocated to initialize ref_by_list
    with open("directory.csv", 'r') as directorycsv:
        # row[0] is parent, row[4] is child
        reader = csv.reader(directorycsv)
        for row in reader:
            entryNum = int(row[1])
            childInode = int(row[4])
            parentInode = int(row[0])
            if childInode != parentInode or parentInode == 2:
                directoryTable[childInode] = parentInode
            if childInode in inodeAllocated:
                inodeAllocated[childInode].ref_by_list.append((parentInode, entryNum))
            else:
                outputFile.write("UNALLOCATED INODE < " + str(childInode) + " > REFERENCED BY DIRECTORY < " + str(parentInode) + " > ENTRY < " + str(entryNum) + " >\n")
            # Check "."
            if entryNum == 0 and childInode != parentInode:
                # if the inode of the "." entry is different than the inode of the containing directory
                entryName = row[5]
                outputFile.write("INCORRECT ENTRY IN < " + str(parentInode) + " > NAME < " + str(entryName) +
                                 " > LINK TO < " + str(childInode) + " > SHOULD BE < " + str(parentInode) + " >\n")
            # Check ".."
            elif entryNum == 1 and childInode != directoryTable[parentInode]:
                entryName = row[5]
                # if the inode of the ".." entry is different than the inode of the inode of the parent of the current directory
                outputFile.write("INCORRECT ENTRY IN < " + str(parentInode) + " > NAME < " + str(entryName) +
                      " > LINK TO < " + str(childInode) + " > SHOULD BE < " + str(directoryTable[parentInode]) + " >\n")    

    for inodeNum in inodeAllocated:
        inode = inodeAllocated[inodeNum]
        linkCount = len(inode.ref_by_list)
        if inode.inodeNumber > 10 and linkCount == 0:
            groupNum = inodeNum / inodesPerGroup
            outputFile.write("MISSING INODE < " + str(inode.inodeNumber) + " > SHOULD BE IN FREE LIST < " + str(inodeBitmapBlocks[groupNum]) + " >\n")
        elif linkCount != inode.nlinks:
            outputFile.write("LINKCOUNT < " + str(inodeNum) + " > IS < " + str(inode.nlinks) + " > SHOULD BE < " + str(linkCount) + " >\n")

    for inodeNum in inodeFreeList:
        if inodeNum in inodeAllocated:
            inode = inodeAllocated[inodeNum]
            outputFile.write("UNALLOCATED INODE < " + str(inodeNum) + " > REFERENCED BY")
            for i in range(len(inode.ref_by_list)):
                outputFile.write(" DIRECTORY < " + str(inode.ref_by_list[i][0]) + " > ENTRY < " + str(inode.ref_by_list[i][1]) + " >")
            outputFile.write("\n")

    for inodeNum in range(11, totalNumInodes+1):
        if (inodeNum not in inodeFreeList) and (inodeNum not in inodeAllocated):
            groupNum = inodeNum / inodesPerGroup
            outputFile.write("MISSING INODE < " + str(inodeNum) + " > SHOULD BE IN FREE LIST < " + str(inodeBitmapBlocks[groupNum]) + " >\n")

    for blockNum in blockAllocated:
        block = blockAllocated[blockNum]
        if len(block.ref_by_list) > 1:
            outputFile.write("MULTIPLY REFERENCED BLOCK < " + str(block.blockNumber) + " > BY")
            for i in range(len(block.ref_by_list)):
                outputFile.write(" INODE < " + str(block.ref_by_list[i][0]) + " > ENTRY < " + str(block.ref_by_list[i][2]) + " >")
            outputFile.write("\n")


    for block in blockFreeList:
        if block in blockAllocated:
            # checks if referenced by an indirect block
            if blockAllocated[block].ref_by_list[0][1] == None:
                outputFile.write("UNALLOCATED BLOCK < " + str(block) + " > REFERENCED BY INODE < " + str(blockAllocated[block].ref_by_list[0][0]) +
                      " > ENTRY < " + str(blockAllocated[block].ref_by_list[0][2]) + " >\n")
            else:
                outputFile.write("UNALLOCATED BLOCK < " + str(block) + " > REFERENCED BY INODE < " + str(blockAllocated[block].ref_by_list[0][0]) +
                      " > ENTRY < " + str(blockAllocated[block].ref_by_list[0][2]) + " > INDIRECT BLOCK < " + str(blockAllocated[block].ref_by_list[0][1]) + " >\n")
    
            
if __name__ == "__main__":
    main()
