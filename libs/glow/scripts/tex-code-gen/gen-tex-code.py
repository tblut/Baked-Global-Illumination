from Cheetah.Template import Template

# http://www.cheetahtemplate.org/docs/users_guide_html/users_guide.html

outCodeDir = "/home/ptrettner/projects/glow-tests/libs/glow/src/glow/objects/"
outTestDir = "/home/ptrettner/projects/glow-tests/src/objects/tex-gen/"

texTypes = {
    "GL_TEXTURE_1D": {
        "shortdesc": "a 1D texture",
        "dims": 1,
        "samplerSuffix": "1D",
    },
    "GL_TEXTURE_2D": {
        "shortdesc": "a 2D texture",
        "dims": 2,
        "samplerSuffix": "2D",
    },
    "GL_TEXTURE_3D": {
        "shortdesc": "a 3D texture",
        "dims": 3,
        "samplerSuffix": "3D",
    },
    "GL_TEXTURE_1D_ARRAY": {
        "shortdesc": "an array of 1D textures",
        "dims": 1,
        "array": True,
        "samplerSuffix": "1D_ARRAY",
    },
    "GL_TEXTURE_2D_ARRAY": {
        "shortdesc": "an array of 2D textures",
        "dims": 2,
        "array": True,
        "samplerSuffix": "2D_ARRAY",
    },
    "GL_TEXTURE_RECTANGLE": {
        "shortdesc": "a rectangular texture",
        "defaultMinFilter": "GL_LINEAR",
        "defaultWrap": "GL_CLAMP_TO_EDGE",
        "hasMipmaps": False,
        "dims": 2,
        "samplerSuffix": "2D_RECT",
    },
    "GL_TEXTURE_CUBE_MAP": {
        "shortdesc": "a CubeMap texture",
        "dims": 2,
        "cubemap": True,
        "samplerSuffix": "CUBE",
    },
    "GL_TEXTURE_CUBE_MAP_ARRAY": {
        "shortdesc": "an array of CubeMap textures",
        "dims": 2,
        "cubemap": True,
        "array": True,
        "samplerSuffix": "CUBE_MAP_ARRAY",
    },
    "GL_TEXTURE_BUFFER": {
        "shortdesc": "a texture buffer",
        "hasMipmaps": False,
        "hasTexParams": False,
        "hasImmutable": False,
        "dims": 1,
        "buffer": True,
        "samplerSuffix": "BUFFER",
    },
    "GL_TEXTURE_2D_MULTISAMPLE": {
        "shortdesc": "a 2D multisampled texture",
        "hasMipmaps": False,
        "dims": 2,
        "hasImmutable": False,
        "isMultisample": True,
        "samplerSuffix": "2D_MULTISAMPLE",
    },
    "GL_TEXTURE_2D_MULTISAMPLE_ARRAY": {
        "shortdesc": "an array of 2D multisampled textures",
        "hasMipmaps": False,
        "dims": 2,
        "array": True,
        "hasImmutable": False,
        "isMultisample": True,
        "samplerSuffix": "2D_MULTISAMPLE_ARRAY",
    },
}


def writeIfDiff(filename, content):
    old_content = ""
    try:
        with open(filename, 'r') as content_file:
            old_content = content_file.read()
    except IOError:
        pass
    if old_content != content:
        open(filename, 'w+').write(content)
        print "  Updating " + filename


def main():
    for type in texTypes:
        print "Generating " + type
        className = type \
            .replace("GL_TEXTURE", "Texture") \
            .replace("_CUBE_MAP", "CubeMap") \
            .replace("_MULTISAMPLE", "Multisample") \
            .replace("_ARRAY", "Array") \
            .replace("_RECTANGLE", "Rectangle") \
            .replace("_BUFFER", "Buffer") \
            .replace("Texture_", "Texture")
        print "  Class Name: " + className

        context = {
            "gltype": type,
            "glbindingtype": type.replace("TEXTURE_", "TEXTURE_BINDING_"),
            "class": className,
            "defaultMinFilter": "GL_NEAREST_MIPMAP_LINEAR",
            "defaultWrap": "GL_REPEAT",
            "hasMipmaps": True,
            "dims": 0,
            "array": False,
            "buffer": False,
            "cubemap": False,
            "hasImmutable": True,
            "hasTexParams": True,
            "isMultisample": False,
        }

        for key in texTypes[type]:
            context[key] = texTypes[type][key]

        # dims
        context["storageDim"] = context["dims"]
        if context["array"]:
            context["storageDim"] += 1
        context["hasClearWorkaround"] = context["dims"] == 2 and not context["cubemap"] and not context["array"]
        context["hasDepthTexture"] = context["dims"] == 2

        # coords
        context["texCoords"] = ["S", "T", "R"][0:context["dims"]]
        context["sizeMember"] = ["Width", "Height", "Depth"][0:context["dims"]]
        context["sizeVars"] = ["width", "height", "depth"][0:context["dims"]]
        context["offsetVars"] = ["x", "y", "z"][0:context["dims"]]
        if context["array"]:
            context["sizeMember"] += ["Layers"]
            context["sizeVars"] += ["layers"]
            context["offsetVars"] += ["l"]
        if context["buffer"]:
            context["sizeMember"] = ["Size"]
            context["sizeVars"] = ["size"]
            context["offsetVars"] = ["offset"]
        context["sizeZeros"] = ["0", "0", "0", "0"][0:len(context["sizeVars"])]
        context["sizeVarsReverse"] = list(reversed(context["sizeVars"]))
        context["sizeParameter"] = ", ".join(["int " + S for S in context["sizeVars"]])
        context["offsetParameter"] = ", ".join(["int " + S for S in context["offsetVars"]])
        context["sizeParameterWithDefault"] = ", ".join(["int " + S + " = 1" for S in context["sizeVars"]])
        context["sizeParameterCall"] = ", ".join(context["sizeVars"])
        context["offsetParameterCall"] = ", ".join(context["offsetVars"])
        context["sizeZeroCall"] = ", ".join(context["sizeZeros"])
        context["sizeCall"] = ", ".join(context["sizeMember"])
        context["sizeMemberVars"] = zip(context["sizeMember"], context["sizeVars"])
        context["sizeDataMember"] = ["Width", "Height", "Depth"][0:context["storageDim"]]
        context["offsetDataMember"] = ["OffsetX", "OffsetY", "OffsetZ"][0:context["storageDim"]]
        context["dimCall"] = ", ".join(map(lambda x: "m" + x, context["sizeMember"]) + ["1", "1", "1"][0:3-len(context["sizeVars"])])

        t = Template(file='texture.hh.tmpl', searchList=context)
        writeIfDiff(outCodeDir + str(className) + ".hh", str(t))

        t = Template(file='texture.cc.tmpl', searchList=context)
        writeIfDiff(outCodeDir + str(className) + ".cc", str(t))

        t = Template(file='texture.test.cc.tmpl', searchList=context)
        writeIfDiff(outTestDir + str(className) + ".cc", str(t))


if __name__ == "__main__":
    main()
