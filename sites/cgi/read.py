import sys

def main():
    print("Enter some text (Ctrl+D to end input on Linux/Mac, Ctrl+Z then Enter on Windows):")
    
    # Read all input from stdin
    data = sys.stdin.read()
    
    print("\nYou entered:")
    print(data)

if __name__ == "__main__":
    main()
