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
    for param_name in data:
        file.write("OBJECT_DECLARE(" + str(param_name) + ");\n")
    
    # Generate structs
    for param_name in data:
        file.write("PARAM_START(" + str(param_name) + ")\n")
        for field_name in data[param_name]:
            field_type = data[param_name][field_name]
            if field_type == "char":
                file.write("\tchar " + str(field_name) + "[MAX_STRING_SIZE];\n")
            elif field_type == "object_id":
                file.write("\tobject_id *" + str(field_name)+";\n")
            elif field_type == "sprite":
                file.write("\tSPRITE_ID " + str(field_name)+";\n")
            elif field_type == "shape":
                file.write("\tpolyshape " + str(field_name)+";\n")
            elif field_type == "emitter":
                file.write("\tEMITTER_ID " + str(field_name)+";\n")
            elif field_type == "sound":
                file.write("\tMix_Chunk *" + str(field_name)+";\n")
            elif field_type == "texture":
                file.write("\tTEXTURE_ID " + str(field_name)+";\n")
            else:
                file.write("\t" + str(field_type) + " " + str(field_name) + ";\n")
        file.write("PARAM_END\n")
    file.write("#include \"../script_data/object_decl.h\"\n")
    file.write("\n#endif /* end of PARAMS_GENERATED_H_ */")
    file.close()

    
def levelParse(file, data):
    file.write("void* parse_generated(cJSON *param, char* type, char *name) \n" + 
               "{\n")
    file.write("  object_id *obj_id = object_by_name(type);\n")
    file.write("  union {\n")
    for param_name in data:
        file.write("      " + papre  + str(param_name) + " " + str(param_name) + ";\n" )
    file.write("  } arg;\n" +
               "  strcpy((char*) &arg, name);\n")
    i = 0
    for index, param_name in enumerate(data):
        if index == 0:
            file.write("  if ")
        else:
            file.write("  } else if ")
        file.write("(obj_id == " +"obj_id_"+ str(param_name) + ") {\n")
        for field_name in data[param_name]:
            field_type = data[param_name][field_name]
            if field_type in ("int" ,"float", "sprite", "object_id", "shape", "emitter", "sound", "texture", "tex_unit"):
                file.write("     arg." + param_name + "." + field_name + " = level_safe_parse_")
                file.write(str(field_type))
                file.write("(param, \"" + field_name +"\");\n")
            elif field_type == "char":
                file.write("     strcpy(arg." + param_name + "." + field_name + ", level_safe_parse_")
                file.write(str(field_type))
                file.write("(param, \"" + field_name +"\"));\n")
    
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
    for index, param_name in enumerate(data):
        if index == 0:
            file.write("  if ")
        else:
            file.write("  } else if ")
        file.write("(obj_id == " +"obj_id_"+ str(param_name) + ") {\n")
        file.write("    obj_param_" + str(param_name) + " * " + str(param_name))
        file.write(" = (obj_param_" + str(param_name) + "*) data;\n")
        for field_name in data[param_name]:
            field_type = data[param_name][field_name]
            if field_type in ("int" ,"float"):
                file.write("    cJSON_AddNumberToObject(param,")
                file.write( "\"" + field_name + "\"," + param_name + "->" + field_name + ");\n")
            elif field_type == "char":
               file.write("    cJSON_AddItemToObject(param,")
               file.write( "\"" + field_name + "\",cJSON_CreateString(" + param_name + "->" + field_name + "));\n")
            elif field_type == "sprite":
               file.write("    cJSON_AddItemToObject(param,")
               file.write( "\"" + field_name + "\",cJSON_CreateString(sprite_get_name(" + param_name + "->" + field_name + ")));\n")
            elif field_type == "shape":
               file.write("    cJSON_AddItemToObject(param,")
               file.write( "\"" + field_name + "\",cJSON_CreateString(" + param_name + "->" + field_name + "->name));\n")
            elif field_type == "object_id":
               file.write("    cJSON_AddItemToObject(param,")
               file.write( "\"" + field_name + "\",cJSON_CreateString(" + param_name + "->" + field_name + "->NAME));\n")
            elif field_type == "emitter":
               file.write("    cJSON_AddItemToObject(param,")
               file.write( "\"" + field_name + "\",cJSON_CreateString(particles_get_name(" + param_name + "->" + field_name + ")));\n")
            elif field_type == "sound":
               file.write("    cJSON_AddItemToObject(param,")
               file.write( "\"" + field_name + "\",cJSON_CreateString(sound_get_name(" + param_name + "->" + field_name + ")));\n")
            elif field_type == "texture":
               file.write("    cJSON_AddItemToObject(param,")
               file.write( "\"" + field_name + "\",cJSON_CreateString(texture_get_name(" + param_name + "->" + field_name + ")));\n")
                        
            
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
