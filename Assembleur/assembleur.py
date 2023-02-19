import os
import re

class Assembler:
    def __init__(self, input_file_path, output='Data/output.bin'):
        self.opcodes = {
            'add': 2, 'addi': 3, 'sub': 4, 'subi': 5, 'mul': 6, 'muli': 7, 'div': 8, 'divi': 9,'and': 10,
            'andi': 11, 'or': 12, 'ori': 13, 'xor': 14, 'xori': 15, 'shl': 16, 'shli': 17,'shr': 18,
            'shri': 19, 'slt': 20, 'slti': 21, 'sle': 22, 'slei': 23, 'seq': 24, 'seqi': 25, 'load': 27,
            'store': 29, 'jmp': 30, 'jmpi': 31, 'braz': 32, 'branz': 33, 'scall': 34, 'stop': 35
        }

        self.registers = ['r{}'.format(i) for i in range(32)]

        self.input_file_path = input_file_path

        # Contenu du fichier source sous forme de liste de lignes
        self.input_file_content = self.read_input_file()

        # Liste des opérations (et de leurs paramètres)
        self.operations = []

        # Dictionnaire des labels du programme source
        self.labels = {}

        self.output_file_path = output

        # Code binaire résultant de la compilation
        self.assembled_code = ''

    def read_input_file(self):
        lines = []
        with open(self.input_file_path, 'r') as file:
            content = file.readlines()
            for line in content:
                # Supprime les commentaires de la ligne
                line = re.sub(r';.*|#.*', '', line)
                # Supprime les espaces inutiles en début et fin de ligne
                line = line.strip()
                # Ajoute seulement les lignes non vides
                if line:
                    lines.append(line)
        return lines

    def get_labels(self):
        labels = {}
        for line in self.input_file_content:
            if ':' not in line:
                continue

            label, newline = line.split(':')
            labels[label] = self.input_file_content.index(line)
            if not newline.strip():
                self.input_file_content.remove(line)
            else:
                newline = newline.strip()
                self.input_file_content[self.input_file_content.index(line)] = newline
            print(f"Label found <{label}> at line {labels[label]}")
        return labels

    # Extrait les operations et leurs paramètres depuis le code source et les retourne dans une liste
    def get_ops(self):
        ops = []
        for line in self.input_file_content:
            if line[0] == ';' or line is None or line == '':
                continue
            if len(line.split()) > 1:
                op, params = line.split(' ', 1) #séparation de l'op et des paramètres
            else :
                op,params = line.split()[0], '' #instruction stop, pas de paramètres
            if op in self.opcodes:
                ops.append([op, params])
                print(f"Op <{op}> found with params <{params}>")
            else:
                print(f"Erreur : <{op}> found for the line <{line}>")
        return ops

    # retourne l'index du registre ou du label
    # retourne la valeur si c'est une valeur
    def get_index(self, param):
        if param in self.registers:  # if it's a register
            return self.registers.index(param)
        elif param in self.labels:  # if it's a label
            return self.labels[param]
        else:  # if it's a value
            return int(param)

    # retourne l'encodage binaire d'une insteuction type 'op param1 param2 param3'
    def operat(self, op, params):
        r1, r2, v = params.split(' ')

        if v not in self.registers and v not in self.labels and self.opcodes.get(f"{op}i") is not None:
            op = self.opcodes[f"{op}i"] << 26
            v = self.get_index(v) & 0x0000FFFF
        else:
            op = self.opcodes[op] << 26
            v = self.get_index(v) << 11

        r1 = self.get_index(r1) << 21
        r2 = self.get_index(r2) << 16
        return op | r1 | r2 | v

    def jmp(self, params):
        ra, rd = params.split()

        if not rd.startswith('r'):
            ra, rd = rd, ra

        if rd not in self.registers and rd not in self.labels:
            op = self.opcodes['jmpi'] << 26
            rd = self.get_index(rd) & 0x000fffff
        else:
            op = self.opcodes['jmp'] << 26
            rd = self.get_index(rd) << 16

        ra = self.get_index(ra) << 21

        return op | rd | ra

    def braz(self, params, op='braz'):
        rs, addr = params.split()
        if rs not in self.registers:
            rs, addr = addr, rs
        op = self.opcodes[op] << 26
        rs = self.get_index(rs) << 21
        addr = self.get_index(addr) & 0x000fffff
        return op | rs | addr

    def branz(self, params):
        return self.braz(params, 'branz')

    def scall(self, param):
        op = self.opcodes['scall'] << 26
        n = self.get_index(param) & 0x03ffffff
        return op | n

    def stop(self,param):
        return self.opcodes['stop'] << 26

    # Convert to hexadecimal
    def bin_to_hex(self, bin):
        # convert to hexadecimal
        res = format(bin, '032b')
        res = ''.join([hex(int(res[i:i + 4], 2))[2:] for i in range(0, len(res), 4)])
        # reverse bytes : 00 00 00 87 -> 87 00 00 00
        return ''.join([res[i:i + 2] for i in range(0, len(res), 2)][::-1])

    # Assemble the code (generate binary), operation by operation
    def assemble(self):
        res = []
        for index , (op, params) in enumerate(self.operations):
            if op not in self.opcodes:
                continue
            if len(params.split()) == 3:
                res.append(self.bin_to_hex(self.operat(op, params)))
            else :
                res.append(self.bin_to_hex(getattr(self, op)(params)))

        return res

    # Formate le résultat pour pouvoir l'écrire dans l'output binaire
    def format_res(self, assembled):
        res = ''
        for instruction in assembled:
            res += instruction
        return res

    def compil(self):
        self.labels = self.get_labels()
        self.operations = self.get_ops()
        res = self.format_res(self.assemble())

        with open(self.output_file_path, 'wb') as f:
            f.write(bytes.fromhex(res))
        print(f"Compilation terminée")

assembler = Assembler("Data/test.txt", "Data/test.bin")
assembler.compil()