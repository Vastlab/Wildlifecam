clc, clear all

totalCorrect = 0;
totalImages = 0;

fileOut = fopen('Exemplar_Scores.txt','wb');

for outerLoop = 1:9
    
    if (outerLoop == 8)
        continue;
    end
    clear gtListFinal
    clear fileListFinal
    
    fileListTemp = fopen(['/home/brian/Desktop/yuiExemplar/ExemplarSVMs/camFiles/cam',int2str(outerLoop),'Files.txt'],'r');
    fileList = textscan(fileListTemp,'%s','delimiter','\n');
    fileListFinal = fileList{1,1};
    
    gtFileTemp = fopen(['/home/brian/Desktop/yuiExemplar/ExemplarSVMs/cam',int2str(outerLoop),'Output.txt'],'r');
    gtList = textscan(gtFileTemp,'%s','delimiter','\n');
    gtListFinal = gtList{1,1};
    
    fclose(gtFileTemp);
    fclose(fileListTemp);
   
    missDir = '/home/brian/Desktop/yuiExemplar/ExemplarSVMs/camMiss/';
    hitDir = '/home/brian/Desktop/yuiExemplar/ExemplarSVMs/camHit/';
    
    missed = 0;
    found = 1;
    
    %get rid of duplicate entries in gtFileTemp
    gtIdx = 1;
    idx = 1;
    firstEntry = gtListFinal{idx,1};
    
    idx = 3;
    secondEntry = gtListFinal{idx,1};
    idx = idx + 2;
    
    firstSlashIdx = strfind(firstEntry,'/');
    firstFileName = firstEntry(firstSlashIdx(end)+1:end);
    
    secondSlashIdx = strfind(secondEntry,'/');
    secondFileName = secondEntry(secondSlashIdx(end)+1:end);
    
    clear gtListOut
    
    if (strcmp(firstFileName,secondFileName) == 0)
        gtListOut{gtIdx,1} = firstFileName;
        gtIdx = gtIdx + 1;
    end
    
    while (idx < size(gtListFinal,1))
        
        firstFileName = secondFileName;
        
        secondEntry = gtListFinal{idx,1};
        
        secondSlashIdx = strfind(secondEntry,'/');
        secondFileName = secondEntry(secondSlashIdx(end)+1:end);
        
        idx = idx + 2;
        
        if (strcmp(firstFileName,secondFileName) == 0)
            gtListOut{gtIdx,1} = firstFileName;
            gtIdx = gtIdx + 1;
        end
        
        
    end
    
    %write last entry
    gtListOut{gtIdx,1} = secondFileName;
    
    totalCorrect = totalCorrect + size(gtListOut,1);
    totalImages = totalImages + size(fileListFinal,1);
    
    camCorrect = size(gtListOut,1);
    camTotal = size(fileListFinal,1);
    
    %compare the fileList entry with gtList entries
    %if match write to hit no match write to miss
    idx = 1;
    gtIdx = 1;
    
    for loop = 1:size(fileListFinal,1)
        
        fileListEntryTemp = fileListFinal{loop,1};
        fileListEntrySlashIdx = strfind(fileListEntryTemp,'/');
        fileListEntry = fileListEntryTemp(fileListEntrySlashIdx(end)+1:end);
        
        if (gtIdx <= size(gtListOut,1))
            gtfileListEntry = gtListOut{gtIdx,1};
        end
        
        %if no match
        if (strcmp(fileListEntry,gtfileListEntry) == 0)
            
            X = imread([fileListEntryTemp]);
            imwrite(X,[missDir,'cam_',int2str(outerLoop),'_',fileListEntry],'Quality',100);
            
        else
            X = imread([fileListEntryTemp]);
            imwrite(X,[hitDir,'cam_',int2str(outerLoop),'_',fileListEntry],'Quality',100);
            gtIdx = gtIdx + 1;
        end
        
        
        
    end
    
    percentFound = camCorrect/camTotal * 100;
    fprintf('Cam: %d Correct: %d Total: %d Percent Correct: %7.3f\n',outerLoop,camCorrect,camTotal,percentFound);
    fprintf(fileOut,'Cam: %d Correct: %d Total: %d Percent Correct: %7.3f\n',outerLoop,camCorrect,camTotal,percentFound);
 
end

fprintf('Total Correct: %d Total: %d Percent Total Correct: %7.3f\n',totalCorrect,totalImages,((totalCorrect/totalImages)*100));
fprintf(fileOut,'Total Correct: %d Total: %d Percent Total Correct: %7.3f\n',totalCorrect,totalImages,((totalCorrect/totalImages)*100));

