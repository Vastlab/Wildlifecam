function SaveModelToText(modelName, filename)

    load(modelName);
    
    fp = fopen(filename, 'wt');
    fprintf(fp,'%d\n',length(model.weight));
    for i = 1 : length(model.weight)
        fprintf(fp,'%f ',model.weight(i)); 
    end
    fprintf(fp,'\n');
    
    for i = 1 : length(model.weight)
        fprintf(fp,'%f ',sqrt(det(model.Sigma(:,:,i))));
    end
    fprintf(fp,'\n');
    
    for i = 1 : length(model.weight)
        for j = 1 : 3
            fprintf(fp,'%f ',model.mu(j,i));
        end
        fprintf(fp,'\n');
    end
    
    for i = 1 : length(model.weight)
        sigmaInv = inv(model.Sigma(:,:,i));
        for j = 1 : 3
            for k = 1 : 3
                fprintf(fp,'%f ',sigmaInv(k,j));
            end
        end
        fprintf(fp,'\n');
    end
    
    fclose(fp);
end