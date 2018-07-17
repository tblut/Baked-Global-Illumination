from Cheetah.Template import Template
import random

# http://www.cheetahtemplate.org/docs/users_guide_html/users_guide.html

outTestDir = "/home/ptrettner/projects/glow-tests/src/other/std140-gen/"

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

def randomType():
    t = random.randint(0, 2)
    if t == 0:
        return random.choice(["int", "uint", "float", "bool", "double"])
    if t == 1:
        return random.choice(["vec", "ivec", "uvec", "bvec", "dvec"]) + str(random.randint(2, 4))
    if t == 2:
        if random.randint(0, 1) == 0:
            return random.choice(["mat", "dmat"]) + str(random.randint(2, 4))
        else:
            return random.choice(["mat", "dmat"]) + str(random.randint(2, 4)) + "x" + str(random.randint(2, 4))

    # TODO: arrays
    # TODO: structs
    raise NotImplementedError


def touchAccess(tName, vName):
    if tName.find("vec") >= 0:
        return "float(" + vName + ".x)"
    if tName.find("mat") >= 0:
        return "float(" + vName + "[0][0])"
    return "float(" + vName + ")"


def main():
    for i in range(0, 30):
        tests = []
        for j in range(0, 30):
            random.seed(12345 + i * 1000 + j)

            fields = []
            for f in range(0, i + 3):
                t = randomType()
                field = {
                    "name": "f" + str(f),
                    "type": t,
                    "access": touchAccess(t, "f" + str(f)),
                }
                fields.append(field)

            test = {
                "name": "Test_" + str(i) + "_" + str(j),
                "fields": fields,
            }

            tests.append(test)

        context = {
            "tests": tests,
            "name": "Test" + str(i)
        }

        t = Template(file='std140.test.cc.tmpl', searchList=context)
        writeIfDiff(outTestDir + "std140gen" + str(i) + ".cc", str(t))


if __name__ == "__main__":
    main()
