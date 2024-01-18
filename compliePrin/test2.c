while(true){
        switch (state)
        {
        case 0:
            if(c == '-') state = 1;
            else if(c == '0') state = 2;
            else if(c >= '1' && c <= '9') state = 3;
            else {
                // 不可能发生
                rollback(c);
                print(MACHERROR,number.c_str());
                return ERROR;
            }
            break;
        case 1:
            if(c == '0') state = 2;
            else if(c >= '1' && c <= '9') state = 3;
            else {
                rollback(c);
                print(MACHERROR,number.c_str());
                return ERROR;
            }
            break;
        case 2:
            if(c == '.') state = 4;
            else if(c == 'e' || c == 'E') state = 6;
            else {
                if(is_endchar(c)){
                    if(c != FILEEOF) number.pop_back();
                    print(NUMBER,number.c_str());
                    rollback(c);
                    return OK;
                }else{
                    while(true){
                        c = getnext(number);
                        if(is_endchar(c)){
                            if(c != FILEEOF) number.pop_back();
                            print(MACHERROR,number.c_str());
                            rollback(c);
                            return ERROR;
                        }
                    }
                }
            }
            break;
        case 3:
            if(c >= '0' && c <= '9') state = 3;
            else if(c == '.') state = 4;
            else if(c == 'e' || c == 'E') state = 6;
            else {
                if(is_endchar(c)){
                    if(c != FILEEOF) number.pop_back();
                    print(NUMBER,number.c_str());
                    rollback(c);
                    return OK;
                }else{
                    while(true){
                        c = getnext(number);
                        if(is_endchar(c)){
                            if(c != FILEEOF) number.pop_back();
                            print(MACHERROR,number.c_str());
                            rollback(c);
                            return ERROR;
                        }
                    }
                }
            }
            break;
        case 4:
            if(c >= '0' && c <= '9') state = 5;
            else {
                rollback(c);
                print(MACHERROR,number.c_str());
                return ERROR;
            }
            break;
        case 5:
            if(c >= '0' && c <= '9') state = 5;
            else if(c == 'e' || c == 'E') state = 6;
            else {
                if(is_endchar(c)){
                    if(c != FILEEOF) number.pop_back();
                    print(NUMBER,number.c_str());
                    rollback(c);
                    return OK;
                }else{
                    while(true){
                        c = getnext(number);
                        if(is_endchar(c)){
                            if(c != FILEEOF) number.pop_back();
                            print(MACHERROR,number.c_str());
                            rollback(c);
                            return ERROR;
                        }
                    }
                }
            }
            break;
        case 6:
            if(c == '+' || c == '-') state = 7;
            else if(c >= '0' && c <= '9') state = 8;
            else {
                rollback(c);
                print(MACHERROR,number.c_str());
                return ERROR;
            }
            break;
        case 7:
            if(c >= '0' && c <= '9') state = 8;
            else {
                rollback(c);
                print(MACHERROR,number.c_str());
                return ERROR;
            }
            break;
        case 8:
            if(c >= '0' && c <= '9') state = 8;
            else {
                if(is_endchar(c)){
                    if(c != FILEEOF) number.pop_back();
                    print(NUMBER,number.c_str());
                    rollback(c);
                    return OK;
                }else{
                    while(true){
                        c = getnext(number);
                        if(is_endchar(c)){
                            if(c != FILEEOF) number.pop_back();
                            print(MACHERROR,number.c_str());
                            rollback(c);
                            return ERROR;
                        }
                    }
                }
            }    
            break;
        default:
            rollback(c);
            return ERROR;
            break;
        }
        c = getnext(number);
    }