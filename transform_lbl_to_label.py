# first python program?
import re

with open('build/invaders.lbl','r') as f:
    read_data = f.read()
    
new_string = re.sub(r'al (.*) \.(.*)', r'{ \n Address: "\1" \n Name: "\2" \n}', read_data )
#print({new_string})
with open('build/invaders.label', 'w') as f:
    f.write(
    """{
  Version: "1"
  Segments: [
    {
      Name: Default
      CodeLabels: [
        """)
    f.write(new_string)
    f.write(']}}')
    f.close()