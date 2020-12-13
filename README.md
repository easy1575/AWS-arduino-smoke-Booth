# AWS-arduino-smoke-Box

서비스개요
공공 흡연부스를 효율적으로 관리하도록 만들었습니다. 구현한 기술로 흡연실 내부 공기질을 파악하여 공기질이 나쁠 시(일정 수치 이상) 정화 시스템이 돌아가며 흡연실 내부를 정화하고, 모든 시스템을 공공 흡연실을 관리하는 사람이 앱으로 관리할 수 있도록 만들었습니다. 

준비물 : 아두이노 MKR WiFi 1010 보드,  GP2Y1010AU0F 센서, DC 모터 모듈 L9110

주요기능 : 미세먼지센서가 담배연기로 측정수가 높아지고 특정 값을 넘는 순간 모터모듈이 ON으로 바뀌며 가동된다.

1. 먼저 아두이노 코드를 작성한다
2. 람다자바를 통해 DB에 디바이스에서 전송하는 값을 저장한다.




    private String persistData(Document document) throws ConditionalCheckFailedException {
        // Epoch Conversion Code: https://www.epochconverter.com/
        SimpleDateFormat sdf = new SimpleDateFormat ( "yyyy-MM-dd HH:mm:ss");
        sdf.setTimeZone(TimeZone.getTimeZone("Asia/Seoul"));
        String timeString = sdf.format(new java.util.Date (document.timestamp*1000));
        
        //DustDensity의 값이 바뀌어야만 저장
        if (document.current.state.reported.Dustdensity.equals(document.previous.state.reported.Dustdensity)) {
                return null;
        }

        return this.dynamoDb.getTable(DYNAMODB_TABLE_NAME)
                .putItem(new PutItemSpec().withItem(new Item().withPrimaryKey("deviceId", document.device)
                        .withLong("time", document.timestamp)
                        .withString("GPS", document.current.state.reported.GPS)
                        .withString("Dustdensity", document.current.state.reported.Dustdensity)
                        .withString("Auto", document.current.state.reported.Auto)
                        .withString("Motor", document.current.state.reported.Motor)
                        .withString("State", document.current.state.reported.State)
                        .withString("timestamp",timeString)))
                .toString();
    }
    private void initDynamoDbClient() {
        AmazonDynamoDB client = AmazonDynamoDBClientBuilder.standard().withRegion("ap-northeast-2").build();

        this.dynamoDb = new DynamoDB(client);
    }
}

class Document {
    public Thing previous;       
    public Thing current;
    public long timestamp;
    public String device;       // AWS IoT에 등록된 사물 이름 
}

class Thing {
    public State state = new State();
    public long timestamp;
    public String clientToken;

    public class State {
        public Tag reported = new Tag();
        public Tag desired = new Tag();

        public class Tag {
            public String GPS;
            public String Dustdensity;
            public String Auto;
            public String Motor;
            public String State;
        }
    }
}





3. RESTAPI를 구현하여 앱으로 디바이스를 제어하게 한다
