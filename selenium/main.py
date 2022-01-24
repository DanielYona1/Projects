from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.by import By
import time
import os

PATH = "C:\Program Files (x86)/chromedriver.exe"
driver = webdriver.Chrome(PATH)
driver.get("https://twitter.com/amit_segal")
if not os.path.exists("test"):
    os.mkdir("test")
men_file = open("test/@_file.txt", "w")
tweet_file = open("test/tweet_file.txt", "w")
tag_file = open("test/tag_file.txt", "w")
counter = 0
tweet_list = []
men_list = []
tag_list = []
while counter < 100:
    time.sleep(2)
    tweet = driver.find_elements(By.XPATH,"//*[contains(@class,'css-901oao r-18jsvk2 r-1k78y06 r-a023e6 r-16dba41 r-rjixqe r-bcqeeo r-bnwqim r-qvutc0 r-1vmecro')]//*[contains(@class,'css-901oao css-16my406 r-1k78y06 r-bcqeeo r-qvutc0')]")
    mentioned = driver.find_elements(By.XPATH,"//*[@class='css-4rbku5 css-18t94o4 css-901oao css-16my406 r-1cvl2hr r-1loqt21 r-poiln3 r-bcqeeo r-qvutc0' and @href]")
    hashtags = driver.find_elements(By.XPATH,"//*[@class='css-4rbku5 css-18t94o4 css-901oao css-16my406 r-1cvl2hr r-1loqt21 r-1k78y06 r-bcqeeo r-qvutc0 r-1vmecro' and @href]")
    for hashtag in hashtags:
        if hashtag.text not in tag_list:
            tag_file.write(hashtag.text)
            tag_file.write("\n")
            tag_list.append(hashtag.text)
    for men in mentioned:
        if men.text not in men_list and men.text[0] == "@":
            men_file.write(men.text)
            men_file.write("\n")
            men_list.append(men.text)
    for tweet_id in tweet:
        if tweet_id.text not in tweet_list:
            counter += 1
            tweet_file.write(str(counter) + " " + tweet_id.text)
            tweet_file.write("\n")
            tweet_file.write("\n")
            tweet_list.append(tweet_id.text)
        if counter == 80:
            break
    elm = driver.find_element(By.TAG_NAME, "html")
    for i in range(20):
        elm.send_keys(Keys.DOWN)


driver.quit()
men_file.close()
tweet_file.close()
tag_file.close()

