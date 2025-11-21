import sys

def djb2_hash(s):
    """Standard DJB2 Hash Algorithm"""
    hash_val = 5381
    for char in s:
        hash_val = ((hash_val << 5) + hash_val) + ord(char)
        hash_val = hash_val & 0xFFFFFFFF # Keep it 32-bit unsigned
    return hash_val

if __name__ == "__main__":
    print("--- Logic Sim Password Manager ---")
    pwd = input("Enter new admin password: ")
    
    hashed = djb2_hash(pwd)
    
    # Write to the file expected by the C app
    with open("admin.secret", "w") as f:
        f.write(str(hashed))
        
    print(f"Success! Hash {hashed} saved to 'admin.secret'.")
    print("You can now delete this script if you want.")