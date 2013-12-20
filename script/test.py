import json
import sys
game = "../src/space/"
papre = "obj_param_"
def generateParams(file, data):
    print "-----------paramsgen.h-------------"
    file.write( "/* GENERATED FILE DO NOT EDIT */\n" +
                "#ifndef PARAMS_GENERATED_H_\n" +
                "#define PARAMS_GENERATED_H_\n" +
                "\n" +
                "#include \"we_object.h\"\n" +
                "#include \"cJSON.h\"\n" +
                "#include \"../level.h\"\n" +
                "#define MAX_STRING_SIZE 256\n\n")
    file.write("void* parse_generated(cJSON *param, char* type, char *name); \n")
    file.write("cJSON * write_generated(object_id * obj_id, void *data, char *type, char *name); \n")
    
    param_list = []
    # OBJECT_DECLARE generate
    for param in data:
        for param_name, value in param.items():
            file.write("OBJECT_DECLARE(" + str(param_name) + ");\n")
    
    # Generate structs
    for param in data:
        for param_name, value in param.items():
            file.write("PARAM_START(" + str(param_name) + ")\n")
            for instance in value:
                for types, name in instance.items():
                    if types == "char":
                        file.write("\tchar " + str(name) + "[MAX_STRING_SIZE];\n")
                    elif types == "object_id":
                        file.write("\tobject_id *" + str(name)+";\n")
                    elif types == "sprite":
                        file.write("\tSPRITE_ID " + str(name)+";\n")
                    elif types == "shape":
                        file.write("\tpolyshape " + str(name)+";\n")
                    else:
                        file.write("\t" + str(types) + " " + str(name) + ";\n")
            file.write("PARAM_END\n")
    file.write("#include \"../script_data/object_decl.h\"\n")
    file.write("\n#endif /* end of PARAMS_GENERATED_H_ */")
    file.close()

    
def levelParse(file, data):
    file.write("void* parse_generated(cJSON *param, char* type, char *name) \n" + 
               "{\n")
    file.write("  object_id *obj_id = object_by_name(type);\n")
    file.write("  union {\n")
    for param in data:
        param_name = param.keys()[0]
        file.write("      " + papre  + str(param_name) + " " + str(param_name) + ";\n" )
    file.write("  } arg;\n" +
               "  strcpy((char*) &arg, name);\n")
    i = 0
    for index, param in enumerate(data):
        (param_name, value) = param.items()[0]
        if index == 0:
            file.write("  if ")
        else:
            file.write("  } else if ")
        file.write("(obj_id == " +"obj_id_"+ str(param_name) + ") {\n")
        for instance in value:
                for types, name in instance.items():
                    if types in ("int" ,"float", "sprite", "object_id", "shape"):
                        file.write("     arg." + param_name + "." + name + " = level_safe_parse_")
                        file.write(str(types))
                        file.write("(param, \"" + name +"\");\n")
                    elif types == "char":
                        file.write("     strcpy(arg." + param_name + "." + name + ", level_safe_parse_")
                        file.write(str(types))
                        file.write("(param, \"" + name +"\"));\n")
            
    file.write("  }\n")
    file.write("  const int paramsize = obj_id->P_SIZE;\n" + 
               "  void * data = calloc(1, paramsize);\n" +
               "  memcpy(data, &arg, paramsize);\n" +
               "  return data;\n")
    file.write("}\n")
    
#def writeParse(file, data):
    

def writeParse(file, data):
    file.write("cJSON * write_generated(object_id *obj_id, void *data,  char *type, char *name) \n" + 
               "{\n")
    file.write("  cJSON *param = cJSON_CreateObject();\n")
    file.write("  cJSON_AddItemToObject(param, \"type\", cJSON_CreateString(type));\n")
    file.write("  cJSON_AddItemToObject(param, \"name\", cJSON_CreateString(name));\n")
   
   
    i = 0
    for index, param in enumerate(data):
        (param_name, value) = param.items()[0]
        if index == 0:
            file.write("  if ")
        else:
            file.write("  } else if ")
        file.write("(obj_id == " +"obj_id_"+ str(param_name) + ") {\n")
        file.write("    obj_param_" + str(param_name) + " * " + str(param_name))
        file.write(" = (obj_param_" + str(param_name) + "*) data;\n")
        for instance in value:
                for types, name in instance.items():
                    if types in ("int" ,"float"):
                        file.write("    cJSON_AddNumberToObject(param,")
                        file.write( "\"" + name + "\"," + param_name + "->" + name + ");\n")
                    elif type == "char":
                       file.write("    cJSON_AddItemToObject(param,")
                       file.write( "\"" + name + "\",cJSON_CreateString(" + param_name + "->" + name + "));\n")
                    elif types == "sprite":
                       file.write("    cJSON_AddItemToObject(param,")
                       file.write( "\"" + name + "\",cJSON_CreateString(sprite_get_name(" + param_name + "->" + name + ")));\n")
                    elif types == "shape":
                       file.write("    cJSON_AddItemToObject(param,")
                       file.write( "\"" + name + "\",cJSON_CreateString(" + param_name + "->" + name + "->name));\n")
                    elif types == "object_id":
                       file.write("    cJSON_AddItemToObject(param,")
                       file.write( "\"" + name + "\",cJSON_CreateString(" + param_name + "->" + name + "->NAME));\n")
                        
            
    file.write("  }\n")
    file.write("  return param;\n")
    file.write("}\n")
    


def generateParse(file, data):
    print "-----------paramsgen.c-------------"
    file.write("/* GENERATED FILE DO NOT CHANGE */\n")
    file.write("\n")
    file.write("#include \"paramsgen.h\"\n")
    file.write("#include \"we.h\"\n")
    levelParse(file,data)
    writeParse(file,data)
    file.close()

f = open(game + 'script_data/hello.json')
cfile = open(game + 'autogen/paramsgen.h','w')
hfile = open(game + 'autogen/paramsgen.c','w')
data = json.load(f)
generateParams(cfile, data)
generateParse(hfile, data)
